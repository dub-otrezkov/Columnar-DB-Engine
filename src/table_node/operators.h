#pragma once

#include "types.h"

namespace JFEngine {

class OPrintIth {
public:
    static std::string Exec(Ti64Column& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }

    static std::string Exec(TStringColumn& col, ui64 i) {
        auto res = col.GetData()[i];
        return res;
    }
};

class OJFPrintIth {
public:
    static std::string Exec(Ti64Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I64ToJFStr(j);
    }

    static std::string Exec(TStringColumn& col, ui64 i) {
        auto res = col.GetData()[i];
        return res;
    }
};

template <typename TOperator, typename... Args>
auto Do(std::shared_ptr<IColumn> col, Args&&... args) {
    switch (col->GetType()) {
        case Ei64Column: {
            return TOperator::Exec(*dynamic_cast<Ti64Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EStringColumn: {
            return TOperator::Exec(*dynamic_cast<TStringColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
    }
}

} // namespace JFEngine