#pragma once

#include <utils/cint/int.h>

#include <bit>
#include <concepts>
#include <cstring>
#include <type_traits>
#include <vector>

#include <immintrin.h>

struct TBitWriter {
    std::vector<ui64> buf;
    ui64 pos = 0;

    void Write(const void* src, ui32 bits) {
        ui64 v = 0;
        std::memcpy(&v, src, (bits + 7) / 8);
        ui64 mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
        v &= mask;
        ui64 lo = pos / 64;
        ui32 sh = pos % 64;
        if (lo + 2 > buf.size()) {
            buf.resize(lo + 2, 0);
        }
        buf[lo] |= v << sh;
        if (sh + bits > 64) {
            buf[lo + 1] |= v >> (64 - sh);
        }
        pos += bits;
    }

    void Put(std::vector<char>& out) {
        auto old_size = out.size();
        ui64 word_count = (pos + 63) / 64;
        out.resize(old_size + word_count * 8);
        std::memcpy(out.data() + old_size, buf.data(), word_count * 8);
    }
};

template <std::integral T, typename Encode>
inline std::vector<char> BitPack(ui64 n, T* values, Encode encode) {
    using U = std::make_unsigned_t<T>;
    U max_encoded = 0;
    for (ui64 i = 0; i < n; i++) {
        U e = encode(values[i]);
        if (e > max_encoded) max_encoded = e;
    }
    ui8 bits = static_cast<ui8>(std::bit_width(static_cast<ui64>(max_encoded)));

    ui64 words = (n * bits + 63) / 64;
    std::vector<ui64> buf(words, 0);
    if (bits > 0) {
        ui64 mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
        for (ui64 i = 0; i < n; i++) {
            ui64 v = static_cast<ui64>(encode(values[i])) & mask;
            ui64 pos = i * bits;
            ui64 lo = pos >> 6;
            ui32 sh = static_cast<ui32>(pos & 63);
            buf[lo] |= v << sh;
            if (sh + bits > 64) {
                buf[lo + 1] |= v >> (64 - sh);
            }
        }
    }

    std::vector<char> out(sizeof(bits) + sizeof(n) + words * sizeof(ui64));
    out[0] = static_cast<char>(bits);
    std::memcpy(out.data() + sizeof(bits), &n, sizeof(n));
    if (words > 0) {
        std::memcpy(out.data() + sizeof(bits) + sizeof(n), buf.data(), words * sizeof(ui64));
    }
    return out;
}

template <std::integral T>
inline std::vector<char> BitPack(ui64 n, T* values) {
    using U = std::make_unsigned_t<T>;
    constexpr ui32 W = sizeof(T) * 8;
    return BitPack(n, values, [](T v) -> U {
        if constexpr (std::is_signed_v<T>) {
            U u = static_cast<U>(v);
            return (u << 1) ^ static_cast<U>(static_cast<std::make_signed_t<U>>(u) >> (W - 1));
        } else {
            return static_cast<U>(v);
        }
    });
}

template <std::integral T>
inline void BitUnpack(size_t packed_size, const char* packed, std::vector<T>& out) {
    using U = std::make_unsigned_t<T>;
    if (packed_size < sizeof(ui8) + sizeof(ui64)) {
        out.clear();
        return;
    }
    ui8 bits = static_cast<ui8>(packed[0]);
    ui64 n;
    std::memcpy(&n, packed + sizeof(bits), sizeof(n));
    out.assign(n, T{});
    if (n == 0) {
        return;
    }
    const char* base = packed + sizeof(bits) + sizeof(n);
    ui64 i = 0;

    if constexpr (sizeof(T) <= 4) {
        if (bits > 0 && n >= 16) {
            const ui32 mask_u = (bits >= 32) ? 0xFFFFFFFFu : ((1u << bits) - 1);
            const __m256i mask_v = _mm256_set1_epi32(static_cast<i32>(mask_u));
            const __m256i one_v = _mm256_set1_epi32(1);
            const __m256i zero_v = _mm256_setzero_si256();

            ui64 simd_end = (n - 8) & ~ui64{7};

            for (; i < simd_end; i += 8) {
                ui64 bit_pos = i * static_cast<ui64>(bits);
                ui64 byte_off = bit_pos >> 3;
                ui32 bit_off = static_cast<ui32>(bit_pos & 7);

                ui32 high_byte_off = (bit_off + 4u * bits) >> 3;

                __m128i lo = _mm_loadu_si128(reinterpret_cast<const __m128i*>(base + byte_off));
                __m128i hi = _mm_loadu_si128(reinterpret_cast<const __m128i*>(base + byte_off + high_byte_off));
                __m256i src = _mm256_set_m128i(hi, lo);

                alignas(32) ui8 shuf_bytes[32];
                alignas(32) i32 shifts[8];

                for (ui32 k = 0; k < 4; k++) {
                    ui32 lane_bit = k * bits + bit_off;
                    ui32 lane_byte = lane_bit >> 3;
                    ui32 lane_shift = lane_bit & 7;
                    shifts[k] = static_cast<i32>(lane_shift);
                    for (ui32 b = 0; b < 4; b++) {
                        ui32 byte_idx = lane_byte + b;
                        shuf_bytes[k * 4 + b] = (byte_idx < 16) ? static_cast<ui8>(byte_idx) : ui8{0xFFu};
                    }
                }
                ui32 hi_bit_off = (bit_off + 4u * bits) & 7;
                for (ui32 k = 4; k < 8; k++) {
                    ui32 lane_bit = (k - 4) * bits + hi_bit_off;
                    ui32 lane_byte = lane_bit >> 3;
                    ui32 lane_shift = lane_bit & 7;
                    shifts[k] = static_cast<i32>(lane_shift);
                    for (ui32 b = 0; b < 4; b++) {
                        ui32 byte_idx = lane_byte + b;
                        shuf_bytes[16 + (k - 4) * 4 + b] = (byte_idx < 16) ? static_cast<ui8>(byte_idx) : ui8{0xFFu};
                    }
                }

                __m256i shuf_mask = _mm256_load_si256(reinterpret_cast<const __m256i*>(shuf_bytes));
                __m256i shuffled = _mm256_shuffle_epi8(src, shuf_mask);
                __m256i shifts_v = _mm256_load_si256(reinterpret_cast<const __m256i*>(shifts));
                __m256i shifted = _mm256_srlv_epi32(shuffled, shifts_v);
                __m256i unpacked = _mm256_and_si256(shifted, mask_v);

                __m256i decoded;
                if constexpr (std::is_signed_v<T>) {
                    __m256i low_bit = _mm256_and_si256(unpacked, one_v);
                    __m256i sign = _mm256_sub_epi32(zero_v, low_bit);
                    __m256i half = _mm256_srli_epi32(unpacked, 1);
                    decoded = _mm256_xor_si256(half, sign);
                } else {
                    decoded = unpacked;
                }

                if constexpr (sizeof(T) == 4) {
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(out.data() + i), decoded);
                } else if constexpr (sizeof(T) == 2) {
                    const __m256i narrow_shuf = _mm256_setr_epi8(
                         0, 1, 4, 5, 8, 9,12,13,-1,-1,-1,-1,-1,-1,-1,-1,
                         0, 1, 4, 5, 8, 9,12,13,-1,-1,-1,-1,-1,-1,-1,-1);
                    __m256i narrowed = _mm256_shuffle_epi8(decoded, narrow_shuf);
                    __m128i lo128 = _mm256_castsi256_si128(narrowed);
                    __m128i hi128 = _mm256_extracti128_si256(narrowed, 1);
                    __m128i packed = _mm_unpacklo_epi64(lo128, hi128);
                    _mm_storeu_si128(reinterpret_cast<__m128i*>(out.data() + i), packed);
                } else {
                    const __m256i narrow_shuf = _mm256_setr_epi8(
                         0, 4, 8,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                         0, 4, 8,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1);
                    __m256i narrowed = _mm256_shuffle_epi8(decoded, narrow_shuf);
                    i32 lo32 = _mm_cvtsi128_si32(_mm256_castsi256_si128(narrowed));
                    i32 hi32 = _mm_cvtsi128_si32(_mm256_extracti128_si256(narrowed, 1));
                    std::memcpy(out.data() + i, &lo32, 4);
                    std::memcpy(out.data() + i + 4, &hi32, 4);
                }
            }
        }
    }

    const ui64 mask = (bits == 64) ? ~0ULL : (bits == 0 ? 0 : (1ULL << bits) - 1);
    for (; i < n; i++) {
        ui64 r = 0;
        if (bits > 0) {
            ui64 pos = i * bits;
            ui64 lo = pos >> 6;
            ui32 sh = static_cast<ui32>(pos & 63);
            ui64 w0;
            std::memcpy(&w0, base + lo * 8, sizeof(w0));
            r = w0 >> sh;
            if (sh + bits > 64) {
                ui64 w1;
                std::memcpy(&w1, base + (lo + 1) * 8, sizeof(w1));
                r |= w1 << (64 - sh);
            }
            r &= mask;
        }
        U u = static_cast<U>(r);
        if constexpr (std::is_signed_v<T>) {
            out[i] = static_cast<T>((u >> 1) ^ -static_cast<U>(u & 1));
        } else {
            out[i] = static_cast<T>(u);
        }
    }
}

struct TBitReader {
    const char* data;
    ui64 pos = 0;

    explicit TBitReader(const char* src) : data(src) {}

    void Read(void* dst, ui32 bits) {
        ui64 lo = pos / 64;
        ui32 sh = pos % 64;
        ui64 w0;
        std::memcpy(&w0, data + lo * 8, sizeof(w0));
        ui64 v = w0 >> sh;
        if (sh + bits > 64) {
            ui64 w1;
            std::memcpy(&w1, data + (lo + 1) * 8, sizeof(w1));
            v |= w1 << (64 - sh);
        }
        ui64 mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
        v &= mask;
        std::memcpy(dst, &v, (bits + 7) / 8);
        pos += bits;
    }
};
