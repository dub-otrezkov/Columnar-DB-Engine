#pragma once

#include "utils/cint/int.h"

#include <memory>
#include <memory_resource>

constexpr ui64 kBufSize = (1 << 25);

class TMemoryArena {
public:
    static TMemoryArena& Instance() {
        static TMemoryArena arena;
        return arena;
    }

    auto& Resource() {
        return res_;
    }

    void Reset() {
        res_.release();
    }

private:
    TMemoryArena() :
        buf_(new char[kBufSize]),
        res_(buf_.get(), kBufSize)
    {}

    std::unique_ptr<char[]> buf_;
    std::pmr::monotonic_buffer_resource res_;
};

inline std::pmr::polymorphic_allocator<> ArenaAlloc() {
    return std::pmr::polymorphic_allocator<>{&TMemoryArena::Instance().Resource()};
}
