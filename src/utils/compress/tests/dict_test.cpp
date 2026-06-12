#include "utils/compress/dict.h"

#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

std::vector<JString> Roundtrip(const std::vector<JString>& in) {
    auto bytes = DictSerialize(in.size(), const_cast<JString*>(in.data()));
    return DictUnserialize(bytes.size(), bytes.data());
}

void ExpectEq(const std::vector<JString>& a, const std::vector<JString>& b) {
    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); i++) {
        EXPECT_EQ(a[i].size(), b[i].size()) << "at " << i;
        EXPECT_EQ(a[i], b[i]) << "at " << i << " a=" << a[i].to_string() << " b=" << b[i].to_string();
    }
}

std::string MakeStr(std::mt19937& rng, size_t len) {
    std::string s;
    s.reserve(len);
    for (size_t i = 0; i < len; i++) {
        s.push_back(static_cast<char>('a' + (rng() % 26)));
    }
    return s;
}

}

TEST(JStringSerialize, AllShortDuplicated) {
    std::vector<JString> in;
    for (int i = 0; i < 200; i++) {
        in.emplace_back(std::string_view("foo"));
        in.emplace_back(std::string_view("bar"));
        in.emplace_back(std::string_view(""));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, MixOfEmptyAndNonEmpty) {
    std::vector<JString> in;
    in.emplace_back(std::string_view(""));
    in.emplace_back(std::string_view("hello"));
    in.emplace_back(std::string_view(""));
    in.emplace_back(std::string_view("a string that exceeds twelve bytes by quite a lot"));
    in.emplace_back(std::string_view(""));
    in.emplace_back(std::string_view("x"));
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, BoundaryShortLong) {
    std::vector<JString> in;
    in.emplace_back(std::string_view("123456789012"));
    in.emplace_back(std::string_view("1234567890123"));
    in.emplace_back(std::string_view("123456789012345"));
    in.emplace_back(std::string_view("12345678901234567890"));
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, DictPathExactly128Uniques) {
    std::vector<JString> in;
    for (int i = 0; i < 128; i++) {
        in.emplace_back(std::string_view("u" + std::to_string(i)));
    }
    for (int i = 0; i < 1000; i++) {
        in.emplace_back(std::string_view("u" + std::to_string(i % 128)));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, DictPath1000Uniques) {
    std::vector<JString> in;
    for (int i = 0; i < 1000; i++) {
        in.emplace_back(std::string_view("longerkey_" + std::to_string(i)));
    }
    for (int i = 0; i < 5000; i++) {
        in.emplace_back(std::string_view("longerkey_" + std::to_string(i % 1000)));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, DictPath5000Uniques) {
    std::vector<JString> in;
    for (int i = 0; i < 5000; i++) {
        in.emplace_back(std::string_view("https://site.example/" + std::to_string(i)));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, DictPath65536Uniques) {
    std::vector<JString> in;
    for (int i = 0; i < 65536; i++) {
        in.emplace_back(std::string_view("v" + std::to_string(i)));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, PlainPathJustOver65536Uniques) {
    std::vector<JString> in;
    for (int i = 0; i < 65537; i++) {
        in.emplace_back(std::string_view("w" + std::to_string(i)));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, PlainPathManyUniques) {
    std::mt19937 rng(42);
    std::vector<JString> in;
    for (int i = 0; i < 2048; i++) {
        in.emplace_back(std::string_view(MakeStr(rng, 5 + (rng() % 60))));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, PlainPathWithEmptiesMixedIn) {
    std::mt19937 rng(7);
    std::vector<JString> in;
    for (int i = 0; i < 2048; i++) {
        if (i % 5 == 0) {
            in.emplace_back(std::string_view(""));
        } else {
            in.emplace_back(std::string_view(MakeStr(rng, 5 + (rng() % 60))));
        }
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, SingleEntry) {
    std::vector<JString> in;
    in.emplace_back(std::string_view("only"));
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, ManyDuplicatesOfLongString) {
    std::vector<JString> in;
    for (int i = 0; i < 2048; i++) {
        in.emplace_back(std::string_view("https://example.com/some/long/path?with=query&and=more"));
    }
    ExpectEq(in, Roundtrip(in));
}

TEST(JStringSerialize, UnicodeContent) {
    std::vector<JString> in;
    in.emplace_back(std::string_view("Привет мир"));
    in.emplace_back(std::string_view("Что это значит"));
    in.emplace_back(std::string_view(""));
    in.emplace_back(std::string_view("samaris пример коробка"));
    for (int i = 0; i < 500; i++) {
        in.emplace_back(std::string_view("Привет мир"));
    }
    ExpectEq(in, Roundtrip(in));
}
