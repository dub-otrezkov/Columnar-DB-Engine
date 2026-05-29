#pragma once

#include <utils/cint/int.h>

#include <bit>
#include <concepts>
#include <cstring>
#include <type_traits>
#include <vector>

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

template <std::integral T, typename Decode>
inline void BitUnpack(size_t packed_size, const char* packed, std::vector<T>& out, Decode decode) {
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
    ui64 mask = (bits == 64) ? ~0ULL : ((bits == 0) ? 0 : ((1ULL << bits) - 1));
    auto* base = packed + sizeof(bits) + sizeof(n);
    for (ui64 i = 0; i < n; i++) {
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
        out[i] = decode(r);
    }
}

template <std::integral T>
inline void BitUnpack(size_t packed_size, const char* packed, std::vector<T>& out) {
    using U = std::make_unsigned_t<T>;
    BitUnpack(packed_size, packed, out, [](U r) -> T {
        if constexpr (std::is_signed_v<T>) {
            return static_cast<T>((r >> 1) ^ -static_cast<U>(r & 1));
        } else {
            return static_cast<T>(r);
        }
    });
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
