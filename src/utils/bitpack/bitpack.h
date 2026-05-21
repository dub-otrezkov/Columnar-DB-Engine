#pragma once

#include <utils/cint/int.h>

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

template <std::integral T>
inline std::vector<char> BitPack(const std::vector<T>& values, ui32 bits) {
    using U = std::make_unsigned_t<T>;
    ui64 n = values.size();
    ui64 words = (n * bits + 63) / 64;
    std::vector<ui64> buf(1 + words, 0);
    buf[0] = n;
    if (bits > 0) {
        ui64 mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
        for (ui64 i = 0; i < n; i++) {
            ui64 v = static_cast<ui64>(static_cast<U>(values[i])) & mask;
            ui64 pos = i * bits;
            ui64 lo = 1 + pos / 64;
            ui32 sh = pos % 64;
            buf[lo] |= v << sh;
            if (sh + bits > 64) {
                buf[lo + 1] |= v >> (64 - sh);
            }
        }
    }
    std::vector<char> out(buf.size() * sizeof(ui64));
    std::memcpy(out.data(), buf.data(), out.size());
    return out;
}

template <std::integral T>
inline void BitUnpack(const char* packed, size_t packed_size, ui32 bits, std::vector<T>& out) {
    if (packed_size < sizeof(ui64)) {
        out.clear();
        return;
    }
    ui64 n;
    std::memcpy(&n, packed, sizeof(n));
    out.assign(n, T{});
    if (bits == 0 || n == 0) {
        return;
    }
    ui64 mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
    const char* base = packed + sizeof(ui64);
    for (ui64 i = 0; i < n; i++) {
        ui64 pos = i * bits;
        ui64 lo = pos / 64;
        ui32 sh = pos % 64;
        ui64 w0;
        std::memcpy(&w0, base + lo * 8, sizeof(w0));
        ui64 r = w0 >> sh;
        if (sh + bits > 64) {
            ui64 w1;
            std::memcpy(&w1, base + (lo + 1) * 8, sizeof(w1));
            r |= w1 << (64 - sh);
        }
        out[i] = static_cast<T>(r & mask);
    }
}

template <std::integral T>
inline void BitUnpack(const std::vector<char>& packed, ui32 bits, std::vector<T>& out) {
    BitUnpack(packed.data(), packed.size(), bits, out);
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
