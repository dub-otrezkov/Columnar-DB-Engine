#pragma once

#include "../types/types.h"

#include <unordered_set>

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

} // namespace JfEngine
