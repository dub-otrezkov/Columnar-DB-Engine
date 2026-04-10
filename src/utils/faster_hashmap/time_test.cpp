#include <gtest/gtest.h>

#include <chrono>
#include <iomanip>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "hashset.h"

namespace {

using Ms = long long;

auto now() { return std::chrono::high_resolution_clock::now(); }

template <typename A, typename B>
Ms elapsed_ms(A start, B end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void print_bench(const char* tag, Ms std_ms, Ms flat_ms) {
    double ratio = std_ms == 0 ? 0.0 : double(flat_ms) / double(std_ms);
    std::cout << std::left  << std::setw(34) << tag
              << "  std="   << std::setw(5)  << std_ms
              << "ms  flat=" << std::setw(5) << flat_ms
              << "ms  ratio=" << std::fixed  << std::setprecision(2) << ratio << "x\n";
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Корректность — contains и дедупликация
// ─────────────────────────────────────────────────────────────────────────────

class SetCorrectnessTest : public ::testing::Test {};

TEST_F(SetCorrectnessTest, InsertReturnsFalseOnDuplicate) {
    BetterHashSet<int> s;
    EXPECT_TRUE(s.insert(42));
    EXPECT_FALSE(s.insert(42));
    EXPECT_FALSE(s.insert(42));
    EXPECT_EQ(s.size(), 1u);
}

TEST_F(SetCorrectnessTest, ContainsCorrect) {
    BetterHashSet<int> s;
    for (int i = 0; i < 100; ++i) s.insert(i);
    for (int i = 0;   i < 100; ++i) EXPECT_TRUE(s.contains(i));
    for (int i = 100; i < 200; ++i) EXPECT_FALSE(s.contains(i));
}

TEST_F(SetCorrectnessTest, NegativeIntKeys) {
    BetterHashSet<int> s;
    for (int i = -200; i <= 200; ++i) s.insert(i);
    EXPECT_EQ(s.size(), 401u);
    for (int i = -200; i <= 200; ++i) EXPECT_TRUE(s.contains(i));
    EXPECT_FALSE(s.contains(201));
    EXPECT_FALSE(s.contains(-201));
}

TEST_F(SetCorrectnessTest, ZeroKey) {
    BetterHashSet<int> s;
    EXPECT_TRUE(s.insert(0));
    EXPECT_TRUE(s.contains(0));
    EXPECT_FALSE(s.contains(1));
}

TEST_F(SetCorrectnessTest, StringKeys) {
    BetterHashSet<std::string> s;
    EXPECT_TRUE(s.insert("hello"));
    EXPECT_TRUE(s.insert("world"));
    EXPECT_FALSE(s.insert("hello"));
    EXPECT_TRUE(s.contains("hello"));
    EXPECT_TRUE(s.contains("world"));
    EXPECT_FALSE(s.contains("missing"));
    EXPECT_EQ(s.size(), 2u);
}

TEST_F(SetCorrectnessTest, EmptyStringKey) {
    BetterHashSet<std::string> s;
    EXPECT_TRUE(s.insert(""));
    EXPECT_TRUE(s.contains(""));
    EXPECT_FALSE(s.insert(""));
    EXPECT_EQ(s.size(), 1u);
}

TEST_F(SetCorrectnessTest, SmallKeyRangeForceCollisions) {
    BetterHashSet<int> s;
    for (int iter = 0; iter < 500; ++iter) s.insert(iter % 11);
    EXPECT_EQ(s.size(), 11u);
    for (int k = 0; k < 11; ++k) EXPECT_TRUE(s.contains(k));
}

TEST_F(SetCorrectnessTest, LargeGrowthAndRehash) {
    const int N = 200000;
    BetterHashSet<int> s;
    for (int i = 0; i < N; ++i) s.insert(i);
    EXPECT_EQ(s.size(), (size_t)N);
    for (int i = 0; i < N; ++i) EXPECT_TRUE(s.contains(i));
    EXPECT_FALSE(s.contains(N));
}

TEST_F(SetCorrectnessTest, ReserveDoesNotLoseElements) {
    BetterHashSet<int> s;
    for (int i = 0; i < 1000; ++i) s.insert(i);
    s.reserve(10000);
    EXPECT_EQ(s.size(), 1000u);
    for (int i = 0; i < 1000; ++i) EXPECT_TRUE(s.contains(i));
}

TEST_F(SetCorrectnessTest, ClearResetsToEmpty) {
    BetterHashSet<int> s;
    for (int i = 0; i < 100; ++i) s.insert(i);
    s.clear();
    EXPECT_TRUE(s.empty());
    for (int i = 0; i < 100; ++i) EXPECT_FALSE(s.contains(i));
    EXPECT_TRUE(s.insert(0));
    EXPECT_EQ(s.size(), 1u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Стресс
// ─────────────────────────────────────────────────────────────────────────────

class SetStressTest : public ::testing::Test {
protected:
    std::mt19937 rng{42};
};

TEST_F(SetStressTest, RandomInsertsMatchStdSet) {
    std::uniform_int_distribution<int> dist(-2000, 2000);
    BetterHashSet<int> s;
    std::unordered_set<int> ref;
    for (int i = 0; i < 100000; ++i) {
        int k = dist(rng);
        ASSERT_EQ(s.insert(k), ref.insert(k).second) << "key=" << k;
    }
    EXPECT_EQ(s.size(), ref.size());
    for (int k : ref) EXPECT_TRUE(s.contains(k));
}

TEST_F(SetStressTest, ContainsAlwaysMatchesStdSet) {
    const int N = 50000;
    std::uniform_int_distribution<int> dist(0, N * 2);
    BetterHashSet<int> s;
    std::unordered_set<int> ref;
    for (int i = 0; i < N; ++i) {
        int k = dist(rng);
        s.insert(k); ref.insert(k);
    }
    for (int i = 0; i <= N * 2; ++i)
        ASSERT_EQ(s.contains(i), (bool)ref.count(i)) << "key=" << i;
}

// ─────────────────────────────────────────────────────────────────────────────
// Бенчмарки
// ─────────────────────────────────────────────────────────────────────────────

class SetBenchmark : public ::testing::Test {
protected:
    static constexpr int N = 5'000'000;
};

// Базовый: вставка уникальных + проверка наличия.
TEST_F(SetBenchmark, SequentialInsertAndContains) {
    long long found_std = 0, found_flat = 0;
    auto t0 = now();
    { std::unordered_set<int> s; s.reserve(N);
      for (int i=0;i<N;++i) s.insert(i); for (int i=0;i<N;++i) found_std+=s.count(i); }
    auto t1 = now();
    { BetterHashSet<int> s; s.reserve(N);
      for (int i=0;i<N;++i) s.insert(i); for (int i=0;i<N;++i) found_flat+=s.contains(i); }
    auto t2 = now();
    print_bench("SET seq insert+contains int", elapsed_ms(t0,t1), elapsed_ms(t1,t2));
    ASSERT_EQ(found_std, found_flat);
}

// 50% промахов — давит на ветку "not found".
TEST_F(SetBenchmark, HighMissRateContains) {
    long long found_std = 0, found_flat = 0;
    auto t0 = now();
    { std::unordered_set<int> s; s.reserve(N/2);
      for (int i=0;i<N/2;++i) s.insert(i*2); for (int i=0;i<N;++i) found_std+=s.count(i); }
    auto t1 = now();
    { BetterHashSet<int> s; s.reserve(N/2);
      for (int i=0;i<N/2;++i) s.insert(i*2); for (int i=0;i<N;++i) found_flat+=s.contains(i); }
    auto t2 = now();
    print_bench("SET high miss-rate contains", elapsed_ms(t0,t1), elapsed_ms(t1,t2));
    ASSERT_EQ(found_std, found_flat);
}

// ~90% дублей — имитирует подсчёт уникальных событий в потоке.
TEST_F(SetBenchmark, DeduplicationHeavyDuplicates) {
    std::mt19937 rng(777);
    std::uniform_int_distribution<int> dist(0, N / 10);
    std::vector<int> stream(N);
    for (auto& k : stream) k = dist(rng);
    long long size_std = 0, size_flat = 0;
    auto t0 = now();
    { std::unordered_set<int> s; s.reserve(N/10);
      for (int k:stream) s.insert(k); size_std=s.size(); }
    auto t1 = now();
    { BetterHashSet<int> s; s.reserve(N/10);
      for (int k:stream) s.insert(k); size_flat=s.size(); }
    auto t2 = now();
    print_bench("SET dedup heavy duplicates", elapsed_ms(t0,t1), elapsed_ms(t1,t2));
    ASSERT_EQ(size_std, size_flat);
}

// Membership filter: строим из N/2 случайных, фильтруем N запросов.
TEST_F(SetBenchmark, MembershipFilter) {
    std::mt19937 rng(0xF00D);
    std::uniform_int_distribution<int> dist(0, N);
    std::vector<int> population(N/2), queries(N);
    for (auto& k : population) k = dist(rng);
    for (auto& k : queries)    k = dist(rng);
    long long found_std = 0, found_flat = 0;
    auto t0 = now();
    { std::unordered_set<int> s(population.begin(), population.end());
      for (int k:queries) found_std+=s.count(k); }
    auto t1 = now();
    { BetterHashSet<int> s; s.reserve(population.size());
      for (int k:population) s.insert(k); for (int k:queries) found_flat+=s.contains(k); }
    auto t2 = now();
    print_bench("SET membership filter", elapsed_ms(t0,t1), elapsed_ms(t1,t2));
    ASSERT_EQ(found_std, found_flat);
}

// 70% contains, 30% insert — горячие ключи [0, 50k].
TEST_F(SetBenchmark, HotKeyMixedWorkload) {
    std::mt19937 rng(0xCAFE);
    std::uniform_int_distribution<int> key_dist(0, 50'000);
    std::uniform_int_distribution<int> op_dist(0, 9);
    long long found_std = 0, found_flat = 0;
    auto t0 = now();
    { std::unordered_set<int> s;
      for (int i=0;i<N;++i) {
          int k=key_dist(rng), op=op_dist(rng);
          if (op<7) found_std+=s.count(k); else s.insert(k); } }
    auto t1 = now();
    { rng.seed(0xCAFE); BetterHashSet<int> s;
      for (int i=0;i<N;++i) {
          int k=key_dist(rng), op=op_dist(rng);
          if (op<7) found_flat+=s.contains(k); else s.insert(k); } }
    auto t2 = now();
    print_bench("SET hot-key mixed workload", elapsed_ms(t0,t1), elapsed_ms(t1,t2));
    ASSERT_EQ(found_std, found_flat);
}

// Рост без reserve — стоимость rehash при дедупликации.
TEST_F(SetBenchmark, GrowthWithoutReserve) {
    long long found_std = 0, found_flat = 0;
    auto t0 = now();
    { std::unordered_set<int> s;
      for (int i=0;i<N;++i) s.insert(i); for (int i=0;i<N;++i) found_std+=s.count(i); }
    auto t1 = now();
    { BetterHashSet<int> s;
      for (int i=0;i<N;++i) s.insert(i); for (int i=0;i<N;++i) found_flat+=s.contains(i); }
    auto t2 = now();
    print_bench("SET growth no reserve", elapsed_ms(t0,t1), elapsed_ms(t1,t2));
    ASSERT_EQ(found_std, found_flat);
}