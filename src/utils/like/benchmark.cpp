#include "kmp_stupid.h"
#include "sse_stupid.h"

#include "utils/faster_vectors/gstring.h"

#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace JfEngine {

static std::vector<std::string> GenStrings(ui64 n, ui64 hit_one_in) {
    static const std::vector<std::string> templates = {
        "https://www.example.com/index.html",
        "https://search.engine.com/q=test+query+long",
        "https://yahoo.com/news/article-12345",
        "https://news.bbc.co.uk/world/europe/2023/article-7891011",
        "https://stackoverflow.com/questions/12345/how-to-use-perf-stat",
        "https://en.wikipedia.org/wiki/Volnitsky_algorithm",
        "https://github.com/ClickHouse/ClickHouse/blob/master/src/Functions/like.cpp",
        "/short",
        "abc",
        "redhotchili",
        "googleAndMore", // 13 chars — needle 'google' straddles prefix/extra boundary
    };

    std::vector<std::string> r;
    r.reserve(n);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> pick(0, (int)templates.size() - 1);
    std::uniform_int_distribution<int> hit(0, (int)hit_one_in - 1);

    for (ui64 i = 0; i < n; i++) {
        std::string s = templates[pick(rng)];
        if (hit(rng) == 0) {
            s += "/google/";
        }
        r.push_back(std::move(s));
    }
    return r;
}

static std::vector<JString> ToJStrings(const std::vector<std::string>& src) {
    std::vector<JString> r;
    r.reserve(src.size());
    for (const auto& s : src) {
        r.emplace_back(s);
    }
    return r;
}

template <typename Finder>
static double Bench(std::vector<JString>& cols, std::string_view needle, ui64 iters, const char* name) {
    using namespace std::chrono;
    ui64 hits = 0;
    auto t0 = steady_clock::now();
    for (ui64 it = 0; it < iters; it++) {
        for (auto& s : cols) {
            if (Finder::Exec(s, needle)) hits++;
        }
    }
    auto t1 = steady_clock::now();
    double ms = duration<double, std::milli>(t1 - t0).count();
    std::cout << "  " << name
              << "  hits=" << hits
              << "  time=" << ms << " ms"
              << std::endl;
    return ms;
}

} // namespace JfEngine

int main() {
    using namespace JfEngine;

    constexpr ui64 N     = 1'000'000;
    constexpr ui64 ITERS = 3;

    auto strs = GenStrings(N, /*hit_one_in=*/10);
    auto cols = ToJStrings(strs);

    std::cout << "rows=" << N << " iterations=" << ITERS << std::endl;

    for (std::string needle : {"google", "github", "Volnitsky", "ClickHouse", "xyzzz"}) {
        std::cout << "\nneedle='" << needle << "' (len " << needle.size() << ")" << std::endl;
        double a = Bench<OKmpSubstrFinder>(cols, needle, ITERS, "kmp");
        double b = Bench<O128SubstrFinder>(cols, needle, ITERS, "sse");
        std::cout << "  speedup: " << (a / b) << "x" << std::endl;
    }
    return 0;
}
