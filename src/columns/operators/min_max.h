#pragma once

#include "../types/types.h"

namespace JfEngine {

struct OMin {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col) {
        using T = typename TCol::ElemType;
        using TRo = typename TCol::ElemTypeRo;

        TRo res = col.GetData().at(0);
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData().at(i));
        }
        return std::make_shared<TCol>(std::vector<T>{res});
    }
};

struct OMinAtIdx {
    template <typename TCol>
    static inline Expected<void> Exec(TCol& col1, ui64 idx, TColumnPtr col2) {
        using T = typename TCol::ElemType;

        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col_f = static_cast<TCol*>(col2.get());
        col1.GetData().at(idx) = std::min(col1.GetData().at(idx), col_f->GetData().at(idx));

        return EError::NoError;
    }
};

struct OMax {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col) {
        using T = typename TCol::ElemType;
        using TRo = typename TCol::ElemTypeRo;

        TRo res = col.GetData().at(0);
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData().at(i));
        }
        return std::make_shared<TCol>(std::vector<T>{res});
    }
};

struct OMaxAtIdx {
    template <typename TCol>
    static inline Expected<void> Exec(TCol& col1, ui64 idx, TColumnPtr col2) {
        using T = typename TCol::ElemType;

        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col_f = static_cast<TCol*>(col2.get());
        col1.GetData().at(idx) = std::max(col1.GetData().at(idx), col_f->GetData().at(idx));

        return EError::NoError;
    }
};

} // namespace JfEngine