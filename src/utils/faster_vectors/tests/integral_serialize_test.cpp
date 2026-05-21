#include "utils/faster_vectors/vector_1d.h"

#include <limits>
#include <random>
#include <vector>

#include <gtest/gtest.h>

template <typename T>
void Roundtrip(const std::vector<T>& in) {
    auto bytes = Serialize<T>(in);
    auto out = Unserialize<T>(bytes);
    ASSERT_EQ(in.size(), out.size());
    for (size_t i = 0; i < in.size(); i++) {
        EXPECT_EQ(in[i], out[i]) << "at " << i;
    }
}

TEST(IntegralSerialize, EmptyVector) {
    Roundtrip(std::vector<i32>{});
    Roundtrip(std::vector<ui64>{});
    Roundtrip(std::vector<i8>{});
}

TEST(IntegralSerialize, AllZeros) {
    Roundtrip(std::vector<i32>(100, 0));
    Roundtrip(std::vector<ui64>(100, 0));
}

TEST(IntegralSerialize, AllSameNonZero) {
    Roundtrip(std::vector<i32>(100, 42));
    Roundtrip(std::vector<i32>(100, -42));
    Roundtrip(std::vector<ui64>(100, 12345));
}

TEST(IntegralSerialize, UnsignedSmallRange) {
    std::vector<ui32> in;
    for (int i = 0; i < 500; i++) in.push_back(i % 8);
    Roundtrip(in);
}

TEST(IntegralSerialize, UnsignedFullRange) {
    std::vector<ui8> in;
    for (int i = 0; i < 256; i++) in.push_back(static_cast<ui8>(i));
    Roundtrip(in);
}

TEST(IntegralSerialize, SignedSmallNegatives) {
    std::vector<i32> in = {-3, -2, -1, 0, 1, 2, 3, -3, 0, 1};
    Roundtrip(in);
}

TEST(IntegralSerialize, SignedFullRangeI8) {
    std::vector<i8> in;
    for (int i = -128; i < 128; i++) in.push_back(static_cast<i8>(i));
    Roundtrip(in);
}

TEST(IntegralSerialize, SignedFullRangeI16) {
    std::vector<i16> in;
    in.push_back(std::numeric_limits<i16>::min());
    in.push_back(std::numeric_limits<i16>::max());
    in.push_back(0);
    in.push_back(-1);
    in.push_back(1);
    Roundtrip(in);
}

TEST(IntegralSerialize, SignedBoundaryI32) {
    std::vector<i32> in;
    in.push_back(std::numeric_limits<i32>::min());
    in.push_back(std::numeric_limits<i32>::max());
    in.push_back(0);
    Roundtrip(in);
}

TEST(IntegralSerialize, UnsignedBoundaryU64) {
    std::vector<ui64> in;
    in.push_back(0);
    in.push_back(std::numeric_limits<ui64>::max());
    in.push_back(1);
    Roundtrip(in);
}

TEST(IntegralSerialize, RandomI32) {
    std::mt19937 rng(42);
    std::vector<i32> in;
    for (int i = 0; i < 2048; i++) {
        in.push_back(static_cast<i32>(rng() % 200) - 100);
    }
    Roundtrip(in);
}

TEST(IntegralSerialize, RandomUi32WithLargeMax) {
    std::mt19937 rng(7);
    std::vector<ui32> in;
    for (int i = 0; i < 2048; i++) {
        in.push_back(rng());
    }
    Roundtrip(in);
}

TEST(IntegralSerialize, BitsExactly8) {
    std::vector<ui16> in;
    for (int i = 0; i < 500; i++) in.push_back(static_cast<ui16>(i % 256));
    Roundtrip(in);
}

TEST(IntegralSerialize, BitsExactly64) {
    std::vector<ui64> in = {0, ~0ULL, 1, ~0ULL >> 1, 12345};
    Roundtrip(in);
}
