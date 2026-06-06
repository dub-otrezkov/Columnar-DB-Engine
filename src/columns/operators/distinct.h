#pragma once

#include "../types/types.h"

#include <boost/unordered/unordered_flat_set.hpp>

#include <type_traits>
#include <unordered_set>
#include <algorithm>

namespace JfEngine {

template <typename T>
using TSet = boost::unordered_flat_set<T>;

struct TDistinctSets {

    std::variant<
        std::vector<TSet<i8>>,
        std::vector<TSet<i16>>,
        std::vector<TSet<i32>>,
        std::vector<TSet<i64>>,
        std::vector<TSet<i128>>,
        std::vector<TSet<ld>>,
        std::vector<TSet<JString>>
    > sets;

    template<typename T>
    TSet<T>& GetSet(ui64 idx) {
        if (!std::holds_alternative<std::vector<TSet<T>>>(sets)) {
            sets.emplace<std::vector<TSet<T>>>();
        }
        auto& vec = std::get<std::vector<TSet<T>>>(sets);
        assert(idx <= vec.size());
        if (vec.size() == idx) {
            vec.resize(idx + 1);
        }
        return vec.at(idx);
    }
};

struct OMultipleDistinctCountDelta {
    template <typename TCol>
    static inline Expected<void> Exec(
        TCol& col1, TColumnPtr& ans, TDistinctSets& sts, std::vector<ui64>* idx) {
        using T = typename TCol::ElemType;
        if (!ans) {
            ans = std::make_shared<Ti64Column>(std::vector<i64>(1, 0));
        }
        auto& v = static_cast<Ti64Column*>(ans.get())->GetData();
        ui64 sz = idx ? *std::max_element(idx->begin(), idx->end()) : 1;
        v.reserve(sz);
        if (!idx) {
            auto& st = sts.GetSet<T>(0);
            if (v.empty()) {
                v.resize(1, 0);
            }
            auto& d = col1.GetData().Vec();
            for (ui64 i = 0; i < d.size(); i++) {
                if (st.insert(d.at(i)).second) {
                    v.at(0)++;
                }
            }
            return EError::NoError;
        }
        auto& id = *idx;
        auto& d = col1.GetData().Vec();
        for (ui64 i = 0; i < d.size(); i++) {
            if (v.size() == id.at(i)) {
                v.resize(id.at(i) + 1, 0);
            }
            if (sts.GetSet<T>(id.at(i)).insert(d.at(i)).second) {
                v.at(id.at(i))++;
            }
        }
        return EError::NoError;
    }

    static inline Expected<void> Exec(
        TDateColumn& col1, TColumnPtr& ans, TDistinctSets& sts, std::vector<ui64>* idx) {
        return EError::UnsupportedErr;
    }

    static inline Expected<void> Exec(
        TTimestampColumn& col1, TColumnPtr& ans, TDistinctSets& sts, std::vector<ui64>* idx) {
        return EError::UnsupportedErr;
    }
};

} // namespace JfEngine
