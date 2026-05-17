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

struct OMultipleMax {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        if (!ans) {
            ans = MakeEmptyColumn(col.GetType()).GetRes();
        } else if (ans->GetType() != col.GetType()) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }
        auto& v = static_cast<TCol*>(ans.get())->GetData();
        if (!idx) {
            if (col.GetSize() == 0) {
                return EError::NoError;
            }
            ui64 start = 0;
            if (v.empty()) {
                v.emplace_back(col.GetData()[0]);
                start = 1;
            }
            auto& d = col.GetData();
            auto& acc = v[0];
            for (ui64 i = start; i < col.GetSize(); i++) {
                acc = std::max(d[i], acc);
            }
            return EError::NoError;
        }
        auto& id = *idx;
        if (col.GetSize() != id.size()) {
            return MakeError<EError::BadArgsErr>("col & idx sizes mismatch");
        }
        for (ui64 i = 0; i < id.size(); i++) {
            assert(id.at(i) <= v.size());
            if (id.at(i) == v.size()) {
                v.emplace_back(col.GetData().at(i));
            } else {
                v.at(id.at(i)) = std::max(col.GetData().at(i), v.at(id.at(i)));
            }
        }

        return EError::NoError;
    }
};

struct OMultipleMin {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        if (!ans) {
            ans = MakeEmptyColumn(col.GetType()).GetRes();
        } else if (ans->GetType() != col.GetType()) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }
        auto& v = static_cast<TCol*>(ans.get())->GetData();
        if (!idx) {
            if (col.GetSize() == 0) {
                return EError::NoError;
            }
            ui64 start = 0;
            if (v.empty()) {
                v.emplace_back(col.GetData()[0]);
                start = 1;
            }
            auto& d = col.GetData();
            auto& acc = v[0];
            for (ui64 i = start; i < col.GetSize(); i++) {
                acc = std::min(d[i], acc);
            }
            return EError::NoError;
        }
        auto& id = *idx;
        if (col.GetSize() != id.size()) {
            return MakeError<EError::BadArgsErr>("col & idx sizes mismatch");
        }
        for (ui64 i = 0; i < id.size(); i++) {
            assert(id.at(i) <= v.size());
            if (id.at(i) == v.size()) {
                v.emplace_back(col.GetData().at(i));
            } else {
                v.at(id.at(i)) = std::min(col.GetData().at(i), v.at(id.at(i)));
            }
        }

        return EError::NoError;
    }
};

} // namespace JfEngine