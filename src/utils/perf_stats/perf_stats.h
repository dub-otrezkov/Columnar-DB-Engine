#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>

namespace JfEngine {

inline thread_local std::vector<uint64_t> tl_child_time_stack;

inline void PushChildFrame() {
    tl_child_time_stack.push_back(0);
}

inline uint64_t PopChildFrame() {
    uint64_t v = tl_child_time_stack.back();
    tl_child_time_stack.pop_back();
    return v;
}

inline void AddToParentChildTime(uint64_t ns) {
    if (!tl_child_time_stack.empty()) {
        tl_child_time_stack.back() += ns;
    }
}

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
            double ms = static_cast<double>(e.total_ns) / 1e6;
            out << std::fixed << std::setprecision(3) << ms << "ms\t" << e.count << '\n';
        }
    }
};

inline TQueryStats* TQueryStats::instance = nullptr;

class TAoEngineTimer {
public:
    TAoEngineTimer(const char* name) {
        if (!TQueryStats::instance) return;
        active_ = true;
        name_ = name;
        frame_before_ = tl_child_time_stack.empty() ? 0 : tl_child_time_stack.back();
        t0_ = std::chrono::steady_clock::now();
    }

    ~TAoEngineTimer() {
        if (!active_) return;
        auto ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - t0_).count());
        uint64_t child_added = tl_child_time_stack.empty() ? 0
            : tl_child_time_stack.back() - frame_before_;
        uint64_t self = ns > child_added ? ns - child_added : 0;
        TQueryStats::instance->Record(name_, self);
        if (!tl_child_time_stack.empty()) tl_child_time_stack.back() += self;
    }

    TAoEngineTimer(const TAoEngineTimer&) = delete;
    TAoEngineTimer& operator=(const TAoEngineTimer&) = delete;

private:
    bool active_ = false;
    const char* name_ = nullptr;
    uint64_t frame_before_ = 0;
    std::chrono::steady_clock::time_point t0_;
};

} // namespace JfEngine
