#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>

namespace JfEngine {

struct TQueryStat {
    std::string name;
    uint64_t total_ns = 0;
    uint64_t count = 0;
};

inline uint64_t ReadProcSelfIoReadBytes() {
    std::ifstream f("/proc/self/io");
    if (!f) {
        return 0;
    }
    std::string line;
    while (std::getline(f, line)) {
        static const std::string kPrefix = "read_bytes:";
        if (line.rfind(kPrefix, 0) == 0) {
            try {
                return std::stoull(line.substr(kPrefix.size()));
            } catch (...) {
                return 0;
            }
        }
    }
    return 0;
}

class TQueryStats {
public:
    static TQueryStats* instance;

    void Record(const char* name, uint64_t ns) {
        auto& stat = stats_[name];
        stat.name = name;
        stat.total_ns += ns;
        stat.count++;
    }

    void RecordDiskRead(uint64_t bytes, uint64_t ns) {
        disk_read_count_++;
        disk_read_bytes_userspace_ += bytes;
        disk_read_ns_ += ns;
    }

    void Print(std::ostream& out) const {
        std::vector<const TQueryStat*> sorted;
        for (const auto& [_, stat] : stats_) {
            sorted.push_back(&stat);
        }
        std::sort(sorted.begin(), sorted.end(), [](const TQueryStat* a, const TQueryStat* b) {
            return a->total_ns > b->total_ns;
        });

        out << "\n=== Query Stats ===\n";
        out << "  " << std::left << std::setw(22) << "worker"
            << std::right << std::setw(12) << "ms" << "  "
            << std::setw(10) << "calls" << '\n';
        for (const auto* e : sorted) {
            double ms = static_cast<double>(e->total_ns) / 1e6;
            out << "  " << std::left << std::setw(22) << e->name
                << std::right << std::fixed << std::setprecision(3) << std::setw(10) << ms << "ms  "
                << std::setw(10) << e->count << '\n';
        }

        uint64_t disk_now = ReadProcSelfIoReadBytes();
        uint64_t disk_delta = disk_now > disk_bytes_at_start_
                                  ? disk_now - disk_bytes_at_start_
                                  : 0;
        double disk_ms = static_cast<double>(disk_read_ns_) / 1e6;
        double disk_us_mb = static_cast<double>(disk_read_bytes_userspace_) / (1024.0 * 1024.0);
        double disk_phys_mb = static_cast<double>(disk_delta) / (1024.0 * 1024.0);
        out << "  " << std::left << std::setw(22) << "DiskRead"
            << std::right << std::fixed << std::setprecision(3) << std::setw(10) << disk_ms << "ms  "
            << std::setw(10) << disk_read_count_ << '\n';
        out << "  " << std::left << std::setw(22) << "DiskBytes(userspace)"
            << std::right << std::fixed << std::setprecision(3) << std::setw(10) << disk_us_mb << "MB" << '\n';
        out << "  " << std::left << std::setw(22) << "DiskBytes(physical)"
            << std::right << std::fixed << std::setprecision(3) << std::setw(10) << disk_phys_mb << "MB" << '\n';
    }

    void Reset() {
        stats_.clear();
        disk_read_ns_ = 0;
        disk_read_count_ = 0;
        disk_read_bytes_userspace_ = 0;
        disk_bytes_at_start_ = ReadProcSelfIoReadBytes();
    }

private:
    std::map<std::string, TQueryStat> stats_;
    uint64_t disk_read_ns_ = 0;
    uint64_t disk_read_count_ = 0;
    uint64_t disk_read_bytes_userspace_ = 0;
    uint64_t disk_bytes_at_start_ = 0;
};

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

class TAoEngineTimer {
public:
    TAoEngineTimer(const char* name) {
        if (!TQueryStats::instance) {
            return;
        }
        active_ = true;
        name_ = name;
        frame_before_ = tl_child_time_stack.empty() ? 0 : tl_child_time_stack.back();
        t0_ = std::chrono::steady_clock::now();
    }

    ~TAoEngineTimer() {
        if (!active_) {
            return;
        }
        auto ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - t0_).count());
        uint64_t child_added = tl_child_time_stack.empty() ? 0 : tl_child_time_stack.back() - frame_before_;
        uint64_t self = ns > child_added ? ns - child_added : 0;
        TQueryStats::instance->Record(name_, self);
        if (!tl_child_time_stack.empty()) {
            tl_child_time_stack.back() += self;
        }
    }

private:
    bool active_ = false;
    const char* name_ = nullptr;
    uint64_t frame_before_ = 0;
    std::chrono::steady_clock::time_point t0_;
};

} // namespace JfEngine
