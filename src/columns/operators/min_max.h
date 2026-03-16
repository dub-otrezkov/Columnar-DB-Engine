#pragma once

#include "../types/types.h"

namespace JFEngine {

struct OMin {
    static Expected<IColumn> Exec(Ti8Column& col) {
        i8 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<Ti8Column>(std::vector<i8>{res});
    }

    static Expected<IColumn> Exec(Ti16Column& col) {
        i16 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<Ti16Column>(std::vector<i16>{res});
    }

    
    static Expected<IColumn> Exec(Ti32Column& col) {
        i32 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<Ti32Column>(std::vector<i32>{res});
    }

    static Expected<IColumn> Exec(Ti64Column& col) {
        i64 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(TDoubleColumn col) {
        ld res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<TDoubleColumn>(std::vector<ld>{res});
    }

    static Expected<IColumn> Exec(TDateColumn& col) {
        TDate res = col.GetData()[0];
        // for (ui64 i = 1; i < col.GetSize(); i++) {
        //     res = std::min(res, col.GetData()[i]);
        // }
        return std::make_shared<TDateColumn>(std::vector<TDate>{res});
    }

    static Expected<IColumn> Exec(TTimestampColumn& col) {
        TTimestamp res = col.GetData()[0];
        // for (ui64 i = 1; i < col.GetSize(); i++) {
        //     res = std::min(res, col.GetData()[i]);
        // }
        return std::make_shared<TTimestampColumn>(std::vector<TTimestamp>{res});
    }

    static Expected<IColumn> Exec(TStringColumn& col) {
        std::string res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<TStringColumn>(std::vector<std::string>{res});
    }
};

struct OVerticalMin {
    static Expected<IColumn> Exec(Ti8Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki8Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti8Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti16Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki16Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti16Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti32Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki32Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti32Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti64Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki64Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti64Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(TDoubleColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::kDoubleColumn) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TDoubleColumn*>(col2.get());
        std::vector<ld> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
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
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::kStringColumn) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TStringColumn*>(col2.get());
        std::vector<std::string> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<TStringColumn>(std::move(ans));
    }
};

struct OMax {
    static Expected<IColumn> Exec(Ti8Column& col) {
        i8 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti8Column>(std::vector<i8>{res});
    }

    static Expected<IColumn> Exec(Ti16Column& col) {
        i16 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti16Column>(std::vector<i16>{res});
    }

    
    static Expected<IColumn> Exec(Ti32Column& col) {
        i32 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti32Column>(std::vector<i32>{res});
    }

    static Expected<IColumn> Exec(Ti64Column& col) {
        i64 res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(TDoubleColumn col) {
        ld res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<TDoubleColumn>(std::vector<ld>{res});
    }

    static Expected<IColumn> Exec(TDateColumn& col) {
        TDate res = col.GetData()[0];
        // for (ui64 i = 1; i < col.GetSize(); i++) {
        //     res = std::min(res, col.GetData()[i]);
        // }
        return std::make_shared<TDateColumn>(std::vector<TDate>{res});
    }

    static Expected<IColumn> Exec(TTimestampColumn& col) {
        TTimestamp res = col.GetData()[0];
        // for (ui64 i = 1; i < col.GetSize(); i++) {
        //     res = std::min(res, col.GetData()[i]);
        // }
        return std::make_shared<TTimestampColumn>(std::vector<TTimestamp>{res});
    }

    static Expected<IColumn> Exec(TStringColumn& col) {
        std::string res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<TStringColumn>(std::vector<std::string>{res});
    }
};

struct OVerticalMax {
    static Expected<IColumn> Exec(Ti8Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki8Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti8Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti16Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki16Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti16Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti32Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki32Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti32Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(Ti64Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::ki64Column) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<Ti64Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(std::move(ans));
    }

    static Expected<IColumn> Exec(TDoubleColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::kDoubleColumn) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TDoubleColumn*>(col2.get());
        std::vector<ld> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
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
        if (col1.GetSize() != col2->GetSize()) {
            return EError::BadArgsErr;
        }
        if (col2->GetType() != EColumn::kStringColumn) {
            return EError::BadArgsErr;
        }
        auto col2_i = static_cast<TStringColumn*>(col2.get());
        std::vector<std::string> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::max(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<TStringColumn>(std::move(ans));
    }
};

} // namespace JFEngine