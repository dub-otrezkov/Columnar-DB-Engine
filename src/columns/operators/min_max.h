#include "../types/types.h"


namespace JFEngine {

struct OMin {
    static Expected<IColumn> Exec(Ti8Column& col) {
        i8 res = INT8_MAX;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti8Column>(std::vector<i8>{res});
    }

    static Expected<IColumn> Exec(Ti16Column& col) {
        i16 res = INT8_MAX;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti16Column>(std::vector<i16>{res});
    }

    
    static Expected<IColumn> Exec(Ti32Column& col) {
        i32 res = INT8_MAX;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti32Column>(std::vector<i32>{res});
    }

    static Expected<IColumn> Exec(Ti64Column& col) {
        i64 res = INT8_MAX;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(TDoubleColumn col) {
        ld res = __DBL_MAX__;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res = std::max(res, col.GetData()[i]);
        }
        return std::make_shared<Ti64Column>(std::vector<ld>{res});
    }

    static Expected<IColumn> Exec(TDateColumn& col) {
        TDate res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<TDateColumn>(std::vector<TDate>{res});
    }

    static Expected<IColumn> Exec(TTimestampColumn& col) {
        TTimestamp res = col.GetData()[0];
        for (ui64 i = 1; i < col.GetSize(); i++) {
            res = std::min(res, col.GetData()[i]);
        }
        return std::make_shared<TDateColumn>(std::vector<TTimestamp>{res});
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
            return MakeError<BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki8Column) {
            return MakeError<BadArgsErr>("no min between i8 and other type");
        }
        auto col2_i = static_cast<Ti8Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(Ti16Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki16Column) {
            return MakeError<BadArgsErr>("no min between i16 and other type");
        }
        auto col2_i = static_cast<Ti16Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(Ti32Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki32Column) {
            return MakeError<BadArgsErr>("no min between i32 and other type");
        }
        auto col2_i = static_cast<Ti32Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(Ti64Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki64Column) {
            return MakeError<BadArgsErr>("no min between i64 and other type");
        }
        auto col2_i = static_cast<Ti64Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(TDoubleColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::kDoubleColumn) {
            return MakeError<BadArgsErr>("no min between double and other type");
        }
        auto col2_i = static_cast<TDoubleColumn*>(col2.get());
        std::vector<ld> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<TDoubleColumn>(ans);
    }

    static Expected<IColumn> Exec(TDateColumn& col, TColumnPtr col2) {
        return MakeError<UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return MakeError<UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TStringColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::kStringColumn) {
            return MakeError<BadArgsErr>("no min between string and other type");
        }
        auto col2_i = static_cast<TStringColumn*>(col2.get());
        std::vector<std::string> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(std::min(col1.GetData()[i], col2_i->GetData()[i]));
        }
        return std::make_shared<TStringColumn>(ans);
    }
};

} // namespace JFEngine