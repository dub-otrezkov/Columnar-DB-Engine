#pragma once

#include "../types/types.h"

#include <algorithm>

namespace JfEngine {

struct OMultipleMax {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        if (!ans) {
            ans = MakeEmptyColumn(col.GetType()).GetRes();
        } else if (ans->GetType() != col.GetType()) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }
        auto& v = static_cast<TCol*>(ans.get())->GetData();
        ui64 sz = idx ? *std::max_element(idx->begin(), idx->end()) : 1;
        v.reserve(sz);
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
        ui64 sz = idx ? *std::max_element(idx->begin(), idx->end()) : 1;
        v.reserve(sz);
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
