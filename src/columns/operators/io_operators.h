#pragma once

#include "../types/types.h"

namespace JFEngine {

struct OPrintIth {
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

struct OJFPrintIth {
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

} // namespace JFEngine