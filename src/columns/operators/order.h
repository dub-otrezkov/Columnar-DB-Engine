#pragma once

#include "operators.h"

#include <algorithm>

namespace JfEngine {

struct OSort {
    template <typename TCol>
    static inline std::vector<i64> Exec(TCol& col, bool rev = false) {
        using T = typename TCol::ElemType;
        std::vector<i64> ans(col.GetSize());
        for (i64 i = 0; i < col.GetSize(); i++) {
            ans[i] = i;
        }
        std::stable_sort(ans.begin(), ans.end(), [&col, rev](i64 i, i64 j) -> bool {
            if (rev) {
                return !(col.GetData().at(i) <= col.GetData().at(j));
            }
            return (col.GetData().at(i) < col.GetData().at(j));
        });
        return ans;
    }
};

struct OMergeSort {
    static constexpr ui64 kUnlimited = -1;
    template <typename TCol>
    static inline Expected<std::vector<i64>> Exec(TCol& col, TColumnPtr other, ui64 limit = kUnlimited, bool rev = false) {
        using T = typename TCol::ElemType;
        if (col.GetType() != other->GetType()) {
            return MakeError<EError::BadArgsErr>("cant merge columns of different type");
        }
        auto& col2 = *static_cast<TCol*>(other.get());

        auto cmp = [&col, &col2, rev](const i64& i, const i64& j) -> bool {
            if (rev) {
                return !(col.GetData().at(i) <= col2.GetData().at(j));
            }
            return (col.GetData().at(i) < col2.GetData().at(j));
        };

        i64 i = 0;
        i64 j = 0;
        auto& data1 = col.GetData();
        auto& data2 = col2.GetData();

        auto order = OSort::Exec(col2, rev);
        std::vector<i64> ans;

        if (limit == kUnlimited) {
            ans.reserve(data1.size() + data2.size());
        } else {
            ans.reserve(limit);
        }

        while (i < data1.size() || j < data2.size()) {
            if (i == data1.size()) {
                ans.push_back(-(order[j] + 1));
                j++;
            } else if (j == data2.size()) {
                ans.push_back(i);
                i++;
            } else if (cmp(i, order[j])) {
                ans.push_back(i);
                i++;
            } else {
                ans.push_back(-(order[j] + 1));
                j++;
            }

            if (ans.size() == limit) {
                break;
            }
        }
        return std::move(ans);
    }
};

struct OApply2 {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, TColumnPtr other, const std::vector<i64>& ids) {
        using T = typename TCol::ElemType;
        if (col.GetType() != other->GetType()) {
            return MakeError<EError::BadArgsErr>("cant merge different columns");
        }
        auto ans = MakeEmptyColumn(col.GetType()).GetRes();
        // std::vector<T> ans;
        // ans.reserve(ids.size());

        // auto& data1 = col.GetData();
        // auto& data2 = static_cast<TCol*>(other.get())->GetData();
        for (auto& i : ids) {
            if (i >= 0) {
                ans->Append(&col, i);
            } else {
                ans->Append(other.get(), -i - 1);
            }
        }

        return std::move(ans);
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
