#pragma once

#include "../types/types.h"

#include <nmmintrin.h>

#include <algorithm>

namespace JfEngine {

struct OPrintIth {
    template <typename TCol>
    static inline std::string Exec(TCol& col, ui64 i) {
        auto res = col.GetData()[i];
        return std::to_string(res);
    }

    static inline std::string Exec(Ti128Column& col, ui64 i) {
        auto res = col.GetData()[i];
        if (res == 0) {
            return "0";
        }
        std::string ans = "";

        bool neg = (res < 0);
        if (neg) {
            ans = "-";
            res = -res;
        }
        while (res > 0) {
            ans += ('0' + res % 10);
            res /= 10;
        }
        std::reverse(ans.begin() + neg, ans.end());
        return ans;
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
        return col.GetData().at(i).to_string();
    }
};

struct OJfPrintOpt {
    template <typename TCol>
    static inline std::vector<char> Exec(TCol& col) {
        return Serialize(col.GetData());
    }
};

} // namespace JfEngine
