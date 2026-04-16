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

struct OJfPrint {
    template <typename TCol>
    static inline void Exec(TCol& col, std::vector<StringVector>& out) {
        using T = typename TCol::ElemType;
        assert(col.GetSize() == out.size() && "cant print column");
        
        for (ui64 i = 0; i < col.GetSize(); i++) {
            out[i].push_back_mcpy(col.GetData().data() + i, sizeof(T));
        }
    }

    static inline void Exec(TStringColumn& col, std::vector<StringVector>& out) {
        assert(col.GetSize() == out.size() && "cant print column");

        for (ui64 i = 0; i < col.GetSize(); i++) {
            out[i].push_back_mcpy(col.GetData().data() + col.GetData().get_pos(i), col.GetData().get_len(i));
        }
    }
};


struct OJfPrintOpt {
    template <typename TCol>
    static inline std::vector<char> Exec(TCol& col) {
        return Serialize(col.GetData());
    }
};

} // namespace JfEngine