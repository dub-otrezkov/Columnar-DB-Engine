#pragma once

#include "../types/types.h"

#include <unordered_set>

namespace JfEngine {

struct ODistinct {
    static Expected<IColumn> Exec(Ti8Column& col1, TColumnPtr col2) {
        if (col2->GetType() != EColumn::ki8Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti8Column*>(col2.get());
        std::vector<i8> ans;
        std::unordered_set<i8> dist;
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
        return std::make_shared<Ti8Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti16Column& col1, TColumnPtr col2) {
        if (col2->GetType() != EColumn::ki16Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti16Column*>(col2.get());
        std::vector<i16> ans;
        std::unordered_set<i16> dist;
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
        return std::make_shared<Ti16Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti32Column& col1, TColumnPtr col2) {
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti32Column*>(col2.get());
        std::vector<i32> ans;
        std::unordered_set<i32> dist;
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
        return std::make_shared<Ti32Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti64Column& col1, TColumnPtr col2) {
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti64Column*>(col2.get());
        std::vector<i64> ans;
        std::unordered_set<i64> dist;
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
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(TDoubleColumn& col1, TColumnPtr col2) {
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TDoubleColumn*>(col2.get());
        std::vector<ld> ans;
        std::unordered_set<ld> dist;
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
        return std::make_shared<TDoubleColumn>(std::move(ans));
    }

    static Expected<IColumn> Exec(TDateColumn& col, TColumnPtr col2) {
        return EError::UnsupportedErr;
    }

    static Expected<IColumn> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return EError::UnsupportedErr;
    }

    static Expected<IColumn> Exec(TStringColumn& col1, TColumnPtr col2) {
        if (col2->GetType() != col1.GetType()) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TStringColumn*>(col2.get());
        std::vector<std::string> ans;
        std::unordered_set<std::string> dist;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            dist.insert(col1.GetData()[i]);
        }
        for (ui64 i = 0; i < col2_i->GetSize(); i++) {
            dist.insert(col2_i->GetData()[i]);
        }
        ans.reserve(dist.size());
        for (auto i : dist) {
            ans.push_back(i);
            // std::cout << ":: " << i << std::endl;
        }
        return std::make_shared<TStringColumn>(std::move(ans));
    }
};

} // namespace JfEngine
