#include "../types/types.h"


namespace JFEngine {

class OSum {
public:
    static Expected<IColumn> Exec(Ti8Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(Ti16Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(Ti32Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(Ti64Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(TDoubleColumn& col) {
        ld res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<TDoubleColumn>(std::vector<ld>{res});
    }

    static Expected<IColumn> Exec(TDateColumn& col) {
        return MakeError<EError::UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TTimestampColumn& col) {
        return MakeError<EError::UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TStringColumn& col) {
        std::string res;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<TStringColumn>(std::vector<std::string>{res});
    }
};

class OVerticalSum {
public:
    static Expected<IColumn> Exec(Ti8Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki8Column) {
            return MakeError<EError::BadArgsErr>("no addition between i8 and other type");
        }
        auto col2_i = static_cast<Ti8Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(Ti16Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki16Column) {
            return MakeError<EError::BadArgsErr>("no addition between i16 and other type");
        }
        auto col2_i = static_cast<Ti16Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(Ti32Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki32Column) {
            return MakeError<EError::BadArgsErr>("no addition between i32 and other type");
        }
        auto col2_i = static_cast<Ti32Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(Ti64Column& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::ki64Column) {
            return MakeError<EError::BadArgsErr>("no addition between i64 and other type");
        }
        auto col2_i = static_cast<Ti64Column*>(col2.get());
        std::vector<i64> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(TDoubleColumn& col1, TColumnPtr col2) {
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

    static Expected<IColumn> Exec(TDateColumn& col, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TTimestampColumn& col1, TColumnPtr col2) {
        return MakeError<EError::UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TStringColumn& col1, TColumnPtr col2) {
        if (col1.GetSize() != col2->GetSize()) {
            return MakeError<EError::BadArgsErr>("wrong size");
        }
        if (col2->GetType() != EColumn::kStringColumn) {
            return MakeError<EError::BadArgsErr>("no addition between string and other type");
        }
        auto col2_i = static_cast<TStringColumn*>(col2.get());
        std::vector<std::string> ans;
        for (ui64 i = 0; i < col1.GetSize(); i++) {
            ans.push_back(col1.GetData()[i] + col2_i->GetData()[i]);
        }
        return std::make_shared<TStringColumn>(ans);
    }
};

} // namespace JFEngine