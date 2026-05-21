#include "utils/bitpack/bitpack.h"

#include <cstdint>
#include <random>
#include <vector>

#include <gtest/gtest.h>

namespace {

i64 SignExtend(ui64 v, ui32 bits) {
    if (bits == 0 || bits == 64) {
        return static_cast<i64>(v);
    }
    ui64 m = 1ULL << (bits - 1);
    return static_cast<i64>((v ^ m) - m);
}

std::vector<char> WriteOnce(const std::vector<std::pair<ui64, ui32>>& items) {
    TBitWriter w;
    for (auto& [v, b] : items) {
        w.Write(&v, b);
    }
    std::vector<char> bytes;
    w.Put(bytes);
    return bytes;
}

}

TEST(BitPack, EmptyWriter) {
    TBitWriter w;
    std::vector<char> bytes;
    w.Put(bytes);
    EXPECT_EQ(bytes.size(), 0u);
}

TEST(BitPack, SingleByteAligned) {
    std::vector<std::pair<ui64, ui32>> items = {
        {0xAB, 8}, {0xCD, 8}, {0xEF, 8},
    };
    auto bytes = WriteOnce(items);
    TBitReader r(bytes.data());
    for (auto& [v, b] : items) {
        ui64 out = 0;
        r.Read(&out, b);
        EXPECT_EQ(out, v) << "bits=" << b;
    }
}

TEST(BitPack, NonAlignedWidths) {
    std::vector<std::pair<ui64, ui32>> items = {
        {1, 1}, {0, 1}, {5, 3}, {0x3FF, 10}, {0x1234, 17}, {0xABCDEF, 24},
    };
    auto bytes = WriteOnce(items);
    TBitReader r(bytes.data());
    for (auto& [v, b] : items) {
        ui64 out = 0;
        r.Read(&out, b);
        EXPECT_EQ(out, v) << "bits=" << b;
    }
}

TEST(BitPack, CrossWordBoundary) {
    std::vector<std::pair<ui64, ui32>> items;
    ui64 mask = (1ULL << 17) - 1;
    for (ui32 i = 0; i < 30; i++) {
        items.emplace_back(((ui64)i * 12345u) & mask, 17);
    }
    auto bytes = WriteOnce(items);
    TBitReader r(bytes.data());
    for (auto& [v, b] : items) {
        ui64 out = 0;
        r.Read(&out, b);
        EXPECT_EQ(out, v);
    }
}

TEST(BitPack, MixedWidths) {
    std::vector<std::pair<ui64, ui32>> items = {
        {0xDEADBEEFCAFEBABEull, 64},
        {0xFF, 8},
        {0xABCD, 16},
        {0x12345678, 32},
        {0x7FFFFFFFFFFFFFFFull, 63},
        {0, 1},
        {1, 1},
    };
    auto bytes = WriteOnce(items);
    TBitReader r(bytes.data());
    for (auto& [v, b] : items) {
        ui64 out = 0;
        r.Read(&out, b);
        EXPECT_EQ(out, v) << "bits=" << b;
    }
}

TEST(BitPack, RandomMany) {
    std::mt19937_64 rng(12345);
    std::vector<std::pair<ui64, ui32>> items;
    items.reserve(2000);
    for (int i = 0; i < 2000; i++) {
        ui32 b = 1 + (rng() % 64);
        ui64 mask = (b == 64) ? ~0ULL : ((1ULL << b) - 1);
        items.emplace_back(rng() & mask, b);
    }
    auto bytes = WriteOnce(items);
    TBitReader r(bytes.data());
    for (auto& [v, b] : items) {
        ui64 out = 0;
        r.Read(&out, b);
        ASSERT_EQ(out, v) << "bits=" << b;
    }
}

TEST(BitPack, SignedRoundtrip) {
    std::vector<std::pair<i32, ui32>> items = {
        {-3, 8}, {127, 8}, {-128, 8},
        {-1, 12}, {-2048, 12}, {2047, 12},
        {-100000, 18}, {131071, 18}, {-131072, 18},
    };
    TBitWriter w;
    for (auto& [v, b] : items) {
        w.Write(&v, b);
    }
    std::vector<char> bytes;
    w.Put(bytes);

    TBitReader r(bytes.data());
    for (auto& [v, b] : items) {
        ui64 raw = 0;
        r.Read(&raw, b);
        EXPECT_EQ(SignExtend(raw, b), v) << "bits=" << b;
    }
}

TEST(BitPack, BitPackRoundtripBasic) {
    std::vector<ui64> in = {0, 1, 2, 3, 4, 5, 6, 7};
    auto packed = BitPack(in, 3);
    std::vector<ui64> out;
    BitUnpack(packed, 3, out);
    EXPECT_EQ(in, out);
}

TEST(BitPack, BitPackEmpty) {
    auto packed = BitPack(std::vector<ui64>{}, 5);
    std::vector<ui64> out;
    BitUnpack(packed, 5, out);
    EXPECT_TRUE(out.empty());
}

TEST(BitPack, BitPackZeroBits) {
    std::vector<ui64> in(50, 0);
    auto packed = BitPack(in, 0);
    std::vector<ui64> out;
    BitUnpack(packed, 0, out);
    ASSERT_EQ(out.size(), in.size());
    for (auto v : out) EXPECT_EQ(v, 0u);
}

TEST(BitPack, BitPackCrossBoundary) {
    std::vector<ui64> in;
    for (ui64 i = 0; i < 100; i++) in.push_back(i * 12345);
    auto packed = BitPack(in, 21);
    std::vector<ui64> out;
    BitUnpack(packed, 21, out);
    ASSERT_EQ(in.size(), out.size());
    ui64 mask = (1ULL << 21) - 1;
    for (size_t i = 0; i < in.size(); i++) {
        EXPECT_EQ(out[i], in[i] & mask) << "at " << i;
    }
}

TEST(BitPack, BitPackFull64) {
    std::vector<ui64> in = {0, ~0ULL, 1, 0xDEADBEEFCAFEBABEULL, 0x8000000000000000ULL};
    auto packed = BitPack(in, 64);
    std::vector<ui64> out;
    BitUnpack(packed, 64, out);
    EXPECT_EQ(in, out);
}

TEST(BitPack, BitPackRandom) {
    std::mt19937_64 rng(123);
    for (ui32 bits = 1; bits <= 64; bits++) {
        std::vector<ui64> in;
        in.reserve(500);
        ui64 mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
        for (int i = 0; i < 500; i++) in.push_back(rng() & mask);
        auto packed = BitPack(in, bits);
        std::vector<ui64> out;
        BitUnpack(packed, bits, out);
        ASSERT_EQ(in, out) << "bits=" << bits;
    }
}

TEST(BitPack, BitPackI32Roundtrip) {
    std::vector<i32> in = {0, 1, 100, 1000, 10000};
    auto packed = BitPack(in, 14);
    std::vector<i32> out;
    BitUnpack(packed, 14, out);
    EXPECT_EQ(in, out);
}

TEST(BitPack, BitPackUi8Roundtrip) {
    std::vector<ui8> in;
    for (int i = 0; i < 256; i++) in.push_back(static_cast<ui8>(i));
    auto packed = BitPack(in, 8);
    std::vector<ui8> out;
    BitUnpack(packed, 8, out);
    EXPECT_EQ(in, out);
}

TEST(BitPack, PutAppendsToExisting) {
    std::vector<char> out = {'a', 'b', 'c'};
    TBitWriter w;
    ui8 v = 0x55;
    w.Write(&v, 8);
    w.Put(out);
    ASSERT_EQ(out.size(), 11u);
    EXPECT_EQ(out[0], 'a');
    EXPECT_EQ(out[1], 'b');
    EXPECT_EQ(out[2], 'c');
    EXPECT_EQ(static_cast<ui8>(out[3]), 0x55);
    for (size_t i = 4; i < 11; i++) {
        EXPECT_EQ(out[i], 0);
    }
}
