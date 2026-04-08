#pragma once

#include "../types/types.h"

namespace JfEngine {

struct OPrintIth {
    template <typename TCol>
    static inline std::string Exec(TCol& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }

    static inline std::string Exec(TDoubleColumn& col, ui64 i) {
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

    static inline std::string Exec(TDateColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return PrintDate(j);
    }

    static inline std::string Exec(TTimestampColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return PrintTimestamp(j);
    }

    static inline std::string Exec(TStringColumn& col, ui64 i) {
        std::string res(col.GetData()[i].length(), '.');
        memcpy(res.data(), col.GetData()[i].data(), res.size());
        return res;
    }
};

struct OJfPrintIth {
    static inline std::string Exec(Ti8Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I8ToJfStr(j);
    }

    static inline std::string Exec(Ti16Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I16ToJfStr(j);
    }

    static inline std::string Exec(Ti32Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I32ToJfStr(j);
    }

    static inline std::string Exec(Ti64Column& col, ui64 i) {
        auto j = col.GetData()[i];
        return I64ToJfStr(j);
    }

    static inline std::string Exec(TDoubleColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return DoubleToJfStr(j);
    }

    static inline std::string Exec(TDateColumn& col, ui64 i) {
        auto j = col.GetData()[i];

        return I16ToJfStr(j.year) + I8ToJfStr(j.month) + I8ToJfStr(j.day);
    }

    static inline std::string Exec(TTimestampColumn& col, ui64 i) {
        auto j = col.GetData()[i];
        return I16ToJfStr(j.date.year) +
               I8ToJfStr(j.date.month) +
               I8ToJfStr(j.date.day) +
               I8ToJfStr(j.hour) +
               I8ToJfStr(j.minute) +
               I8ToJfStr(j.second);
    }

    static inline std::string Exec(TStringColumn& col, ui64 i) {
        std::string res(col.GetData()[i].length(), '.');
        memcpy(res.data(), col.GetData()[i].data(), res.size());
        return res;
    }
};

struct OJfPrint {
    static inline std::vector<std::string> Exec(Ti8Column& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = I8ToJfStr(col.GetData()[i]);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(Ti16Column& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = I16ToJfStr(col.GetData()[i]);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(Ti32Column& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = I32ToJfStr(col.GetData()[i]);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(Ti64Column& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = I64ToJfStr(col.GetData()[i]);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(TDoubleColumn& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = DoubleToJfStr(col.GetData()[i]);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(TDateColumn& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            auto j = col.GetData()[i];
            ans[i] = I16ToJfStr(j.year) + I8ToJfStr(j.month) + I8ToJfStr(j.day);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(TTimestampColumn& col) {
        std::vector<std::string> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            auto j = col.GetData()[i];
            ans[i] = I16ToJfStr(j.date.year) +
                    I8ToJfStr(j.date.month) +
                    I8ToJfStr(j.date.day) +
                    I8ToJfStr(j.hour) +
                    I8ToJfStr(j.minute) +
                    I8ToJfStr(j.second);
        }
        return std::move(ans);
    }

    static inline std::vector<std::string> Exec(TStringColumn& col) {
        std::vector<std::string> ans(col.GetSize());
        // auto j = col.GetData()[i];
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[i];
        }
        return std::move(ans);
    }
};


struct OJfPrintOpt {
    template <typename TCol>
    static inline std::vector<char> Exec(TCol& col) {
        return Serialize(col.GetData());
    }
};

} // namespace JfEngine