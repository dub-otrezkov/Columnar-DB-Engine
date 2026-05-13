#pragma once

#include "utils/cint/int.h"

#include <sys/mman.h>

class TStringHeap {
public:
    inline static TStringHeap& Instance() {
        static TStringHeap heap;
        return heap;
    }

    inline static char* Allocate(ui64 size) {
        auto& heap = Instance();
        if (!heap.pages_ || heap.pages_->capacity < size) {
            if (size <= kHugePageSize) {
                NewHugePage();
            } else {
                NewCustomPage(size);
            }
        }
        auto ans = heap.pages_->addr;
        heap.pages_->addr += size;
        heap.pages_->capacity -= size;
        return ans;
    }

    inline static void Free() {
        auto& heap = Instance();
        while (heap.pages_) {
            auto cp = heap.pages_;
            heap.pages_ = heap.pages_->next;
            free(cp->base);
            delete cp;
        }
        heap.pages_ = nullptr;
    }

    ~TStringHeap() {
        Free();
    }

private:
    static constexpr ui64 kHugePageSize = 2 * 1024 * 1024;

    inline static void NewHugePage() {
        auto& heap = Instance();

        void* mem = malloc(kHugePageSize);
        if (!mem) {
            throw std::runtime_error("oom");
        }
        madvise(mem, kHugePageSize, MADV_HUGEPAGE);

        page* p = new page;
        p->capacity = kHugePageSize;
        p->addr = static_cast<char*>(mem);
        p->base = mem;
        p->next = heap.pages_;
        heap.pages_ = p;
    }

    inline static void NewCustomPage(ui64 size) {
        std::cout << "doesnt fit in huge page - requested " << size << " bytes" << std::endl;
        auto& heap = Instance();

        void* mem = malloc(size);
        if (!mem) {
            throw std::runtime_error("oom");
        }
        madvise(mem, size, MADV_HUGEPAGE);

        page* p = new page;
        p->capacity = size;
        p->base = mem;
        p->addr = static_cast<char*>(mem);
        if (heap.pages_) {
            p->next = heap.pages_->next;
            heap.pages_->next = p;
        } else {
            p->next = nullptr;
            heap.pages_ = p;
        }
    }

    struct page {
        ui64 capacity;
        char* addr;
        void* base;
        page* next;
    };

    page* pages_;
};