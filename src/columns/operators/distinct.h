#pragma once

#include "../types/types.h"

#include <boost/unordered/unordered_flat_set.hpp>

#include <type_traits>
#include <unordered_set>

namespace JfEngine {

// FOR DISTINCTS

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
        auto& vec = std::get<std::vector<TSet<T>>>(slots);
        if (vec.size() <= idx) {
            vec.resize(idx + 1);
        }
        return vec.at(idx);
    }
};

// struct ODistinctStreamV {
//     template <typename TCol>
//     static inline Expected<TColumnPtr> Exec(TCol& col1, TDistinctSets& st) {
//         using T = typename TCol::ElemTypeRo;
//         std::vector<T> ans;
//         for (ui64 i = 0; i < col1.GetSize(); i++) {
//             if (st.GetSet<T>().insert(col1.GetData().at(i)).second) {
//                 ans.push_back(col1.GetData().at(i));
//             }
//         }
//         return std::make_shared<TCol>(std::move(ans));
//     }

//     static inline Expected<TColumnPtr> Exec(TDateColumn& col, TDistinctSets& st) {
//         return EError::UnsupportedErr;
//     }

//     static inline Expected<TColumnPtr> Exec(TTimestampColumn& col1, TDistinctSets& st) {
//         return EError::UnsupportedErr;
//     }
// };

struct ODistinctCountDelta {
    template <typename TCol>
    static inline Expected<ui64> Exec(TCol& col1, TSet<typename TCol::ElemType>& st) {
        using T = typename TCol::ElemType;
        ui64 ans = 0;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            if (st.insert(col1.GetData().at(i)).second) {
                ans++;
            }
        }
        return ans;
    }

    static inline Expected<ui64> Exec(TDateColumn& col, TDistinctSets& st) {
        return EError::UnsupportedErr;
    }

    static inline Expected<ui64> Exec(TTimestampColumn& col1, TDistinctSets& st) {
        return EError::UnsupportedErr;
    }
};

} // namespace JfEngine
