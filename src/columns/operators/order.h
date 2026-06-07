#pragma once

#include "operators.h"

#include <algorithm>

namespace JfEngine {

struct OMergeSort {
    static constexpr ui64 kUnlimited = -1;
};

struct OApply2 {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, TColumnPtr other, const std::vector<i64>& ids) {
        if (col.GetType() != other->GetType()) {
            return MakeError<EError::BadArgsErr>("cant merge different columns");
        }
        auto& col2 = *static_cast<TCol*>(other.get());

        auto res = std::make_shared<TCol>();
        for (auto& i : ids) {
            if (i >= 0) {
                res->LoadFrom(col, i);
            } else {
                res->LoadFrom(col2, -i - 1);
            }
        }

        return res;
    }
};

struct TIntComparator {
    using FnPtr = i64 (*)(void*, i64, i64);
    void* ctx = nullptr;
    FnPtr fn = nullptr;

    inline i64 operator()(i64 i, i64 j) const {
        return fn(ctx, i, j);
    }
};

struct TIntComparator2 {
    using FnPtr = i64 (*)(void*, void*, i64, i64);
    void* lhs = nullptr;
    void* rhs = nullptr;
    FnPtr fn = nullptr;

    inline i64 operator()(i64 i, i64 j) const {
        return fn(lhs, rhs, i, j);
    }
};

struct OCmp {
    template <typename TCol>
    static inline TIntComparator Exec(TCol& col) {
        return TIntComparator{
            &col,
            +[](void* p, i64 i, i64 j) -> i64 {
                auto& data = static_cast<TCol*>(p)->GetData();
                const auto& a = data[i];
                const auto& b = data[j];
                if (a < b) {
                    return 1;
                }
                if (a == b) {
                    return 0;
                }
                return -1;
            }
        };
    }
};

struct OCmpDiffCol {
    template <typename TCol>
    static inline TIntComparator2 Exec(TCol& self, TColumnPtr other) {
        if (self.GetType() != other->GetType()) {
            return TIntComparator2{};
        }
        return TIntComparator2{
            &self,
            other.get(),
            +[](void* lp, void* rp, i64 i, i64 j) -> i64 {
                auto& l = static_cast<TCol*>(lp)->GetData();
                auto& r = static_cast<TCol*>(rp)->GetData();
                const auto& a = l[i];
                const auto& b = r[j];
                if (a < b) {
                    return 1;
                }
                if (a == b) {
                    return 0;
                }
                return -1;
            }
        };
    }
};

} // namespace JfEngine
