#pragma once

#include "../types/types.h"

namespace JfEngine {

struct OMin {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col) {
        using T = typename TCol::ElemType;
        using TRo = typename TCol::ElemTypeRo;

        TRo res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::allocate_shared<TCol>(ArenaAlloc(), std::vector<T>{res});
    }
};

struct OVerticalMin {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col1, TColumnPtr col2) {
        using T = typename TCol::ElemType;

        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TCol*>(col2.get());
        std::vector<T> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::allocate_shared<TCol>(ArenaAlloc(), std::move(ans));
    }
};

struct OMax {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col) {
        using T = typename TCol::ElemType;
        using TRo = typename TCol::ElemTypeRo;

        TRo res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::allocate_shared<TCol>(ArenaAlloc(), std::vector<T>{res});
    }
};

struct OVerticalMax {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col1, TColumnPtr col2) {
        using T = typename TCol::ElemType;

        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TCol*>(col2.get());
        std::vector<T> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::allocate_shared<TCol>(ArenaAlloc(), std::move(ans));
    }
};

} // namespace JfEngine