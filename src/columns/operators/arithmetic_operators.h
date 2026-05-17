#pragma once

#include "../types/types.h"

#include "utils/logger/logger.h"

#include <algorithm>

namespace JfEngine {

struct OSum {
    template <typename T>
    static inline Expected<TColumnPtr> Exec(T& col) {
        i128 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += static_cast<i128>(col.GetData()[i]);
        }
        // JF_LOG(nullptr, "sum " << static_cast<i64>(res) << " of size " << col.GetSize() << std::endl);
        return std::make_shared<Ti128Column>(std::vector<i128>{res});
    }

    static inline Expected<TColumnPtr> Exec(TDoubleColumn& col) {
        ld res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<TDoubleColumn>(std::vector<ld>{res});
    }

    static inline Expected<TColumnPtr> Exec(TDateColumn& col) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col) {
        return MakeError<EError::UnsupportedErr>();
    }
};

struct OVerticalSum {
    template<typename T>
    static inline Expected<TColumnPtr> Exec(T& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != col1.GetType()) {
            return MakeError<EError::BadArgsErr>(
                "no addition between i8 and other type " + std::to_string(col1.GetType()) + " " + std::to_string(col2->GetType()));
        }
        auto col2_i = static_cast<T*>(col2.get());
        std::vector<i128> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<Ti128Column>(ans);
    }

    static inline Expected<TColumnPtr> Exec(TDoubleColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::kDoubleColumn) {
            return MakeError<EError::BadArgsErr>("no addition between double and other type");
        }
        auto col2_i = static_cast<TDoubleColumn*>(col2.get());
        std::vector<ld> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<TDoubleColumn>(ans);
    }

    static inline Expected<TColumnPtr> Exec(TDateColumn& col, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }
};

struct OVerticalSub {
    template <typename T>
    static inline Expected<TColumnPtr> Exec(T& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != col1.GetType()) {
            return MakeError<EError::BadArgsErr>("no addition between i8 and other type");
        }
        auto col2_i = static_cast<T*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] - col2_i->GetData()[i]);
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static inline Expected<TColumnPtr> Exec(TDoubleColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::kDoubleColumn) {
            return MakeError<EError::BadArgsErr>("no addition between double and other type");
        }
        auto col2_i = static_cast<TDoubleColumn*>(col2.get());
        std::vector<ld> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] - col2_i->GetData()[i]);
        }
        return std::make_shared<TDoubleColumn>(ans);
    }

    static inline Expected<TColumnPtr> Exec(TDateColumn& col, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }
};

struct OMakeAvg {
    static inline Expected<TColumnPtr> Exec(TDateColumn& col, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    template<typename T>
    static inline Expected<TColumnPtr> Exec(T& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        std::vector<i128> ans;
        auto& cnt = static_cast<Ti64Column*>(col2.get())->GetData();
        ans.reserve(cnt.size());
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.emplace_back(col1.GetData().at(i) / cnt.at(i));
        }
        return std::make_shared<Ti128Column>(ans);
    }
};

struct OLength {
    static inline Expected<TColumnPtr> Exec(TStringColumn& col) {
        std::vector<i64> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData().at(i).size();
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    template<typename T>
    static inline Expected<TColumnPtr> Exec(T& col) {
        return MakeError<EError::UnsupportedErr>();
    }
};

struct OAddConst {
    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col, const std::string& s) {
        return EError::UnsupportedErr;
    }

    static inline Expected<TColumnPtr> Exec(TDateColumn& col, const std::string& s) {
        return EError::UnsupportedErr;
    }

    template<typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, const std::string& s) {
        typename TCol::ElemType add;
        try {
            if constexpr (std::is_same_v<TCol, TStringColumn>) {
                add = s;
            } else {
                add = static_cast<typename TCol::ElemType>(std::stoi(s));
            }
        } catch (...) {
            return EError::BadArgsErr;
        }
        auto res = std::static_pointer_cast<TCol>(MakeEmptyColumn(col.GetType()).GetRes());
        res->GetData() = col.GetData();
        for (auto& v : res->GetData()) {
            v += add;
        }
        return res;
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col, const std::string& s) {
        return MakeError<EError::UnsupportedErr>();
    }
};

struct OAddAtIdx {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& col, ui64 idx, TColumnPtr s) {

        // JF_LOG(nullptr, "addition at " << idx << std::endl);

        if (col.GetType() != s->GetType()) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }
        if (s->GetSize() != 1) {
            return MakeError<EError::BadArgsErr>("size not 1");
        }
        col.GetData().at(idx) += static_cast<TCol*>(s.get())->GetData().at(0);

        return EError::NoError;
    }

    static inline Expected<void> Exec(TTimestampColumn& col, ui64 idx, TColumnPtr s) {
        return EError::UnsupportedErr;
    }

    static inline Expected<void> Exec(TDateColumn& col, ui64 idx, TColumnPtr s) {
        return EError::UnsupportedErr;
    }

    static inline Expected<void> Exec(TStringColumn& col, ui64 idx, TColumnPtr s) {
        return MakeError<EError::UnsupportedErr>();
    }
};

struct OMultipleAdder {
    template<CIntegralColumn TCol>
    static inline Expected<void> Exec(TCol& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        if (!ans) {
            ans = MakeEmptyColumn(EColumn::ki128Column).GetRes();
        } else if (ans->GetType() != EColumn::ki128Column) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }

        if (!idx) {
            auto& v = static_cast<Ti128Column*>(ans.get())->GetData();
            if (v.empty()) {
                v.push_back(0);
            }
            auto& d = col.GetData();
            auto& acc = v[0];
            for (ui64 i = 0; i < col.GetSize(); i++) {
                acc += d[i];
            }
            return EError::NoError;
        }
        if (col.GetSize() != idx->size()) {
            return MakeError<EError::BadArgsErr>("col & idx sizes mismatch");
        }
        auto& v =  static_cast<Ti128Column*>(ans.get())->GetData();
        ui64 sz = idx ? *std::max_element(idx->begin(), idx->end()) : 1;
        v.reserve(sz);
        auto& id = *idx;
        for (ui64 i = 0; i < id.size(); i++) {
            assert(id.at(i) <= v.size());
            if (id.at(i) == v.size()) {
                v.emplace_back(col.GetData().at(i));
            } else {
                v.at(id.at(i)) += col.GetData().at(i);
            }
        }

        return EError::NoError;
    }

    template<typename TCol>
    static inline Expected<void> Exec(TCol& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        assert(1 == 0);
        return EError::NoError;
    }
};

struct OSubConst {
    static inline Expected<TColumnPtr> Exec(TTimestampColumn& col, const std::string& s) {
        return EError::UnsupportedErr;
    }

    static inline Expected<TColumnPtr> Exec(TDateColumn& col, const std::string& s) {
        return EError::UnsupportedErr;
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col, const std::string& s) {
        return EError::UnsupportedErr;
    }

    template<typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, const std::string& s) {
        typename TCol::ElemType add;
        try {
            add = static_cast<typename TCol::ElemType>(std::stoi(s));
        } catch (...) {
            return EError::BadArgsErr;
        }
        auto res = std::dynamic_pointer_cast<TCol>(MakeEmptyColumn(col.GetType()).GetRes());
        res->GetData() = col.GetData();
        for (auto& v : res->GetData()) {
            v -= add;
        }
        return res;
    }
};

} // namespace JfEngine