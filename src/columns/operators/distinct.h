#pragma once

#include "../types/types.h"

#include <unordered_set>
#include <type_traits>

namespace JfEngine {

struct ODistinct {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col1, TColumnPtr col2) {
        using T = typename TCol::ElemTypeRo;

        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TCol*>(col2.get());
        std::vector<T> ans;
        std::unordered_set<T> dist;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            dist.insert(col1.GetData()[i]);
        }
        for (ui64 i = 0; i < col2_i->GetSize(); i++) {
            dist.insert(col2_i->GetData()[i]);
        }
        ans.reserve(dist.size());
        for (auto i : dist) {
            ans.push_back(i);
        }
        return std::make_shared<TCol>(std::move(ans));
    }

    static inline Expected<IColumn> Exec(TDateColumn& col, TColumnPtr col2) {
        return EError::UnsupportedErr;
    }

    static inline Expected<IColumn> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return EError::UnsupportedErr;
    }
};

// FOR DISTINCTS

struct TDistinctSets {
    std::optional<BetterHashSet<i8>> i8_set;
    std::optional<BetterHashSet<i16>> i16_set;
    std::optional<BetterHashSet<i32>> i32_set;
    std::optional<BetterHashSet<i64>> i64_set;
    std::optional<BetterHashSet<ld>> ld_set;
    std::optional<BetterHashSet<std::string>> string_set;

    template<typename T>
    BetterHashSet<T>& GetSet() {
        if constexpr (std::is_same_v<T, i8>) {
            if (!i8_set) {
                i8_set = BetterHashSet<i8>{};
            }
            return *i8_set;
        } else if constexpr (std::is_same_v<T, i16>) {
            if (!i16_set) {
                i16_set = BetterHashSet<i16>{};
            }
            return *i16_set;
        } else if constexpr (std::is_same_v<T, i32>) {
            if (!i32_set) {
                i32_set = BetterHashSet<i32>{};
            }
            return *i32_set;
        } else if constexpr (std::is_same_v<T, i64>) {
            if (!i64_set) {
                i64_set = BetterHashSet<i64>{};
            }
            return *i64_set;
        } else if constexpr (std::is_same_v<T, ld>) {
            if (!ld_set) {
                ld_set = BetterHashSet<ld>{};
            }
            return *ld_set;
        } else {
            if (!string_set) {
                string_set = BetterHashSet<std::string>{};
            }
            return *string_set;
        }
    }
};

struct ODistinctStreamV {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col1, TDistinctSets& st) {
        using T = typename TCol::ElemTypeRo;
        std::vector<T> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            if (st.GetSet<T>().insert(col1.GetData()[i])) {
                ans.push_back(col1.GetData()[i]);
            }
        }
        return std::make_shared<TCol>(std::move(ans));
    }

    static inline Expected<IColumn> Exec(TDateColumn& col, TDistinctSets& st) {
        return EError::UnsupportedErr;
    }

    static inline Expected<IColumn> Exec(TTimestampColumn& col1, TDistinctSets& st) {
        return EError::UnsupportedErr;
    }
};

} // namespace JfEngine
