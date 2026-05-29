#include "utils/compress/delta.h"

#include <cstdint>
#include <limits>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(Delta, EmptyUi64) {
    std::vector<ui64> in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
    EXPECT_TRUE(out.empty());
}

TEST(Delta, SingleElement) {
    std::vector<ui64> in = {42};
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
    std::vector<ui64> expected = {42};
    EXPECT_EQ(expected, out);
}

TEST(Delta, AllEqualValues) {
    std::vector<ui64> in(100, 12345);
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
    std::vector<ui64> expected(100, 12345);
    EXPECT_EQ(expected, out);
}

TEST(Delta, SmallRangePositive) {
    std::vector<ui32> in = {1000, 1003, 1001, 1010, 1005, 1002};
    std::vector<ui32> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui32>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, MonotonicIncreasing) {
    std::vector<ui64> in;
    for (ui64 i = 0; i < 1000; i++) {
        in.push_back(1'000'000 + i * 7);
    }
    std::vector<ui64> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, SignedNegativeRange) {
    std::vector<i32> in = {-100, -50, 0, 50, 100, -75, 25};
    std::vector<i32> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<i32>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, SignedAllNegative) {
    std::vector<i64> in = {-1000, -999, -1005, -1001, -1003};
    std::vector<i64> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<i64>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, SignedAtExtremes) {
    std::vector<i32> in = {std::numeric_limits<i32>::min(),
                           std::numeric_limits<i32>::min() + 1,
                           std::numeric_limits<i32>::max() - 1,
                           std::numeric_limits<i32>::max()};
    std::vector<i32> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<i32>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, UnsignedAtMax) {
    std::vector<ui64> in = {std::numeric_limits<ui64>::max() - 5,
                            std::numeric_limits<ui64>::max() - 2,
                            std::numeric_limits<ui64>::max()};
    std::vector<ui64> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, Ui8Roundtrip) {
    std::vector<ui8> in;
    for (int i = 0; i < 256; i++) in.push_back(static_cast<ui8>(i));
    std::vector<ui8> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui8>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, RandomUi64) {
    std::mt19937_64 rng(42);
    std::vector<ui64> in;
    in.reserve(5000);
    ui64 base = 100'000'000'000ULL;
    for (int i = 0; i < 5000; i++) {
        in.push_back(base + (rng() % 1024));
    }
    std::vector<ui64> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, RandomI32) {
    std::mt19937 rng(7);
    std::vector<i32> in;
    in.reserve(2000);
    for (int i = 0; i < 2000; i++) {
        in.push_back(static_cast<i32>(rng()) % 1000 - 500);
    }
    std::vector<i32> expected = in;
    auto packed = DeltaSerialize(in.size(), in.data());
    auto out = DeltaUnserialize<i32>(packed.size(), packed.data());
    EXPECT_EQ(expected, out);
}

TEST(Delta, RandomAllBitWidths) {
    std::mt19937_64 rng(123);
    for (ui32 spread_bits = 1; spread_bits <= 32; spread_bits++) {
        std::vector<ui64> in;
        in.reserve(300);
        ui64 spread_mask = (1ULL << spread_bits) - 1;
        ui64 base = 1'000'000ULL;
        for (int i = 0; i < 300; i++) {
            in.push_back(base + (rng() & spread_mask));
        }
        std::vector<ui64> expected = in;
        auto packed = DeltaSerialize(in.size(), in.data());
        auto out = DeltaUnserialize<ui64>(packed.size(), packed.data());
        ASSERT_EQ(expected, out) << "spread_bits=" << spread_bits;
    }
}

TEST(Delta, CompressionVsBitPack) {
    std::vector<ui64> in;
    for (ui64 i = 0; i < 1000; i++) {
        in.push_back(1'000'000'000ULL + i);
    }
    std::vector<ui64> in_copy = in;

    auto bitpacked = BitPack(in.size(), in.data());
    auto delta_packed = DeltaSerialize(in_copy.size(), in_copy.data());

    EXPECT_LT(delta_packed.size(), bitpacked.size())
        << "delta=" << delta_packed.size() << " bitpack=" << bitpacked.size();
}
