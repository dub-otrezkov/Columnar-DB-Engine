#pragma once

#include "../types/types.h"

namespace JFEngine {

class OPrintIth {
public:
    static std::string Exec(Ti8Column& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }
    static std::string Exec(Ti16Column& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }

    static std::string Exec(Ti32Column& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }

    static std::string Exec(Ti64Column& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }

    static std::string Exec(TDoubleColumn& col, ui64 i) {
        auto res = col.GetData()[i];
        auto ans = std::to_string(res);
        while (!ans.empty() && ans.back() == '0') {
            ans.pop_back();
        }
        if (ans.back() == '.') {
            ans.pop_back();
        }
        return ans;
    }

    static std::string Exec(TDateColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return PrintDate(j);
    }

    static std::string Exec(TTimestampColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return PrintTimestamp(j);
    }

    static std::string Exec(TStringColumn& col, ui64 i) {
        auto res = col.GetData()[i];
        return res;
    }
};

class OJFPrintIth {
public:
    static std::string Exec(Ti8Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I8ToJFStr(j);
    }

    static std::string Exec(Ti16Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I16ToJFStr(j);
    }

    static std::string Exec(Ti32Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I32ToJFStr(j);
    }

    static std::string Exec(Ti64Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I64ToJFStr(j);
    }

    static std::string Exec(TDoubleColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return DoubleToJFStr(j);
    }

    static std::string Exec(TDateColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return I16ToJFStr(j.year) + I8ToJFStr(j.month) + I8ToJFStr(j.day);
    }

    static std::string Exec(TTimestampColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return I16ToJFStr(j.date.year) +
               I8ToJFStr(j.date.month) +
               I8ToJFStr(j.date.day) +
               I8ToJFStr(j.hour) +
               I8ToJFStr(j.minute) +
               I8ToJFStr(j.second);
    }

    static std::string Exec(TStringColumn& col, ui64 i) {
        auto res = col.GetData()[i];
        return res;
    }
};

template <typename TOperator, typename... Args>
auto Do(TColumnPtr col, Args&&... args) {
    switch (col->GetType()) {
        case Ei8Column: {
            return TOperator::Exec(*static_cast<Ti8Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case Ei16Column: {
            return TOperator::Exec(*static_cast<Ti16Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case Ei32Column: {
            return TOperator::Exec(*static_cast<Ti32Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case Ei64Column: {
            return TOperator::Exec(*static_cast<Ti64Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EDoubleColumn: {
            return TOperator::Exec(*static_cast<TDoubleColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EDateColumn: {
            return TOperator::Exec(*static_cast<TDateColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case ETimestampColumn: {
            return TOperator::Exec(*static_cast<TTimestampColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EStringColumn: {
            return TOperator::Exec(*static_cast<TStringColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
    }
    throw std::runtime_error("bad column type");
}

} // namespace JFEngine