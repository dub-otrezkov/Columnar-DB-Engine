#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

namespace JfEngine {

struct TQueryStats {
    struct TEntry {
        std::string_view name;
        uint64_t total_ns = 0;
        uint64_t count = 0;
    };

    static TQueryStats* instance;

    std::vector<TEntry> entries;

    void Record(std::string_view name, uint64_t ns) {
        for (auto& e : entries) {
            if (e.name == name) {
                e.total_ns += ns;
                e.count++;
                return;
            }
        }
        entries.push_back({name, ns, 1});
    }

    void Reset() { entries.clear(); }

    void Print(std::ostream& out = std::cout) const {
        if (entries.empty()) return;
        auto sorted = entries;
        std::sort(sorted.begin(), sorted.end(), [](const TEntry& a, const TEntry& b) {
            return a.total_ns > b.total_ns;
        });
        out << "  worker                ms       calls\n";
        for (const auto& e : sorted) {
            out << "  " << e.name;
            for (int i = static_cast<int>(e.name.size()); i < 22; ++i) out << ' ';
            out << e.total_ns / 1'000'000 << "ms\t" << e.count << '\n';
        }
    }
};

inline TQueryStats* TQueryStats::instance = nullptr;

} // namespace JfEngine
