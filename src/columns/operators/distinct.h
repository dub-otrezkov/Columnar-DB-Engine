#pragma once

#include "../types/types.h"

#include <boost/unordered/unordered_flat_set.hpp>

#include <type_traits>
#include <unordered_set>

namespace JfEngine {

// FOR DISTINCTS

struct TDistinctSets {
    template <typename T>
    using TSet = boost::unordered_flat_set<T>;

    std::optional<TSet<i8>> i8_set;
    std::optional<TSet<i16>> i16_set;
    std::optional<TSet<i32>> i32_set;
    std::optional<TSet<i64>> i64_set;
    std::optional<TSet<i128>> i128_set;
    std::optional<TSet<ld>> ld_set;
    std::optional<TSet<JString>> string_set;

    template<typename T>
    TSet<T>& GetSet() {
        if constexpr (std::is_same_v<T, i8>) {
            if (!i8_set) {
                i8_set = TSet<i8>{};
            }
            return *i8_set;
        } else if constexpr (std::is_same_v<T, i16>) {
            if (!i16_set) {
                i16_set = TSet<i16>{};
            }
            return *i16_set;
        } else if constexpr (std::is_same_v<T, i32>) {
            if (!i32_set) {
                i32_set = TSet<i32>{};
            }
            return *i32_set;
        } else if constexpr (std::is_same_v<T, i64>) {
            if (!i64_set) {
                i64_set = TSet<i64>{};
            }
            return *i64_set;
        } else if constexpr (std::is_same_v<T, i128>) {
            if (!i128_set) {
                i128_set = TSet<i128>{};
            }
            return *i128_set;
        } else if constexpr (std::is_same_v<T, ld>) {
            if (!ld_set) {
                ld_set = TSet<ld>{};
            }
            return *ld_set;
        } else {
            if (!string_set) {
                string_set = TSet<JString>{};
            }
            return *string_set;
        }
    }
};

struct ODistinctStreamV {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col1, TDistinctSets& st) {
        using T = typename TCol::ElemTypeRo;
        std::vector<T> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            if (st.GetSet<T>().insert(col1.GetData().at(i)).second) {
                ans.push_back(col1.GetData().at(i));
            }
        }
        return std::make_shared<TCol>(std::move(ans));
    }

    static inline Expected<TColumnPtr> Exec(TDateColumn& col, TDistinctSets& st) {
        return EError::UnsupportedErr;
    }

    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col1, TDistinctSets& st) {
        return EError::UnsupportedErr;
    }
};

} // namespace JfEngine
