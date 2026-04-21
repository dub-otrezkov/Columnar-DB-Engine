#include "hashmap.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iomanip>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// Предполагается, что ваш хеш-мап лежит здесь

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
// Корректность
// ─────────────────────────────────────────────────────────────────────────────

class MapCorrectnessTest : public ::testing::Test {};

TEST_F(MapCorrectnessTest, BasicInsertAndRetrieve) {
    BetterHashMap<int, std::string> m;
    m.emplace(1, "one");
    m[2] = "two";
    
    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m.at(1), "one");
    EXPECT_EQ(m[2], "two");
}

TEST_F(MapCorrectnessTest, OperatorSquareBracketsValuePersistence) {
    BetterHashMap<int, int> m;
    m[10] = 100;
    m[10] += 50;
    EXPECT_EQ(m[10], 150);
    EXPECT_EQ(m.size(), 1u);
}

TEST_F(MapCorrectnessTest, EmplaceDoesNotOverwrite) {
    BetterHashMap<int, int> m;
    m.emplace(1, 100);
    m.emplace(1, 200); // Не должно перезаписать 100
    EXPECT_EQ(m[1], 100);
}

TEST_F(MapCorrectnessTest, IterationWithStructuredBindings) {
    BetterHashMap<int, int> m;
    for(int i = 0; i < 10; ++i) m[i] = i * 10;

    int sum_keys = 0;
    int sum_values = 0;
    // Используем auto&& или auto, чтобы прокси-объект ValueProxy корректно привязался
    for (auto&& [key, value] : m) {
        sum_keys += key;
        sum_values += value;
    }
    EXPECT_EQ(sum_keys, 45);
    EXPECT_EQ(sum_values, 450);
}

// ─────────────────────────────────────────────────────────────────────────────
// Бенчмарки
// ─────────────────────────────────────────────────────────────────────────────

class MapBenchmark : public ::testing::Test {
protected:
    static constexpr int N = 5'000'000;
};

// Последовательная вставка и чтение
TEST_F(MapBenchmark, SequentialInsertAndAccess) {
    long long sum_std = 0, sum_flat = 0;
    
    auto t0 = now();
    {
        std::unordered_map<int, int> m;
        m.reserve(N);
        for (int i = 0; i < N; ++i) m[i] = i;
        for (int i = 0; i < N; ++i) sum_std += m[i];
    }
    auto t1 = now();
    {
        BetterHashMap<int, int> m;
        m.reserve(N);
        for (int i = 0; i < N; ++i) m[i] = i;
        for (int i = 0; i < N; ++i) sum_flat += m[i];
    }
    auto t2 = now();
    
    print_bench("MAP seq insert+access int", elapsed_ms(t0, t1), elapsed_ms(t1, t2));
    ASSERT_EQ(sum_std, sum_flat);
}

// Поиск отсутствующих ключей (промахи)
TEST_F(MapBenchmark, LookupMisses) {
    long long found_std = 0, found_flat = 0;
    
    auto t0 = now();
    {
        std::unordered_map<int, int> m;
        m.reserve(N/2);
        for (int i = 0; i < N/2; ++i) m[i * 2] = i;
        for (int i = 0; i < N; ++i) if (m.count(i)) found_std++;
    }
    auto t1 = now();
    {
        BetterHashMap<int, int> m;
        m.reserve(N/2);
        for (int i = 0; i < N/2; ++i) m[i * 2] = i;
        for (int i = 0; i < N; ++i) if (m.contains(i)) found_flat++;
    }
    auto t2 = now();
    
    print_bench("MAP lookup misses (50%)", elapsed_ms(t0, t1), elapsed_ms(t1, t2));
    ASSERT_EQ(found_std, found_flat);
}

// Сценарий накопления (Aggregation): много обновлений по существующим ключам
TEST_F(MapBenchmark, HeavyUpdateAggregation) {
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(0, 100'000); // Узкий диапазон ключей
    std::vector<int> keys(N);
    for (auto& k : keys) k = dist(rng);

    auto t0 = now();
    {
        std::unordered_map<int, int> m;
        for (int k : keys) m[k]++;
    }
    auto t1 = now();
    {
        BetterHashMap<int, int> m;
        for (int k : keys) m[k]++;
    }
    auto t2 = now();

    print_bench("MAP heavy update aggregation", elapsed_ms(t0, t1), elapsed_ms(t1, t2));
}

// Работа со строковыми ключами
TEST_F(MapBenchmark, StringKeysBenchmark) {
    const int SN = 1'000'000;
    std::vector<std::string> strings;
    for(int i = 0; i < SN; ++i) strings.push_back("key_prefix_" + std::to_string(i));

    auto t0 = now();
    {
        std::unordered_map<std::string, int> m;
        for (const auto& s : strings) m[s] = 1;
    }
    auto t1 = now();
    {
        BetterHashMap<std::string, int> m;
        for (const auto& s : strings) m[s] = 1;
    }
    auto t2 = now();

    print_bench("MAP string keys insert", elapsed_ms(t0, t1), elapsed_ms(t1, t2));
}