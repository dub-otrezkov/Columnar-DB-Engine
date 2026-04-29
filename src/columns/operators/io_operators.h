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
        std::string res(col.GetData().at(i).length(), '.');
        memcpy(res.data(), col.GetData().at(i).data(), res.size());
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

struct OEqualRow {
    template <typename TCol>
    static inline bool Exec(TCol& col, ui64 row, const StringVector& key, ui64 col_idx) {
        using T = typename TCol::ElemType;

        if (key.get_len(col_idx) != sizeof(T)) {
            return false;
        }

        T lhs, rhs;
        std::memcpy(&lhs, col.GetData().data() + row, sizeof(T));
        std::memcpy(&rhs, key.data() + key.get_pos(col_idx), sizeof(T));
        return (lhs == rhs);
    }

    static inline bool Exec(TStringColumn& col, ui64 row, const StringVector& key, ui64 col_idx) {
        ui64 len = col.GetData().get_len(row);
        if (len != key.get_len(col_idx)) {
            return false;
        }

        return std::memcmp(
            col.GetData().data() + col.GetData().get_pos(row),
            key.data() + key.get_pos(col_idx),
            len
        ) == 0;
    }
};

struct OHashInto {
    template <typename TCol>
    static inline void Exec(TCol& col, std::vector<ui64>& hashes) {
        using T = typename TCol::ElemType;
        const T* data = col.GetData().data();
        const ui64 sz = col.GetSize();

        for (ui64 i = 0; i < sz; i++) {
            if (i > 0 && col.GetData().at(i - 1) == col.GetData().at(i)) {
                hashes[i] = hashes[i - 1];
                continue;
            }

            const char* p = reinterpret_cast<const char*>(data + i);
            size_t len = sizeof(T);
            ui64 h = hashes[i];

            while (len >= 8) {
                ui64 chunk;
                std::memcpy(&chunk, p, 8);
                h = _mm_crc32_u64(h, chunk);
                p += 8; len -= 8;
            }
            while (len > 0) {
                h = _mm_crc32_u8((ui32)h, *p++);
                len--;
            }
            hashes[i] = h;
        }
    }

    static inline void Exec(TStringColumn& col, std::vector<ui64>& hashes) {
        const auto& sv = col.GetData();
        const ui64 sz = col.GetSize();

        for (ui64 i = 0; i < sz; i++) {
            const char* p = sv.data() + sv.get_pos(i);
            ui64 len = sv.get_len(i);
            ui64 h = hashes[i];

            h = _mm_crc32_u64(h, len);

            while (len >= 8) {
                ui64 chunk;
                std::memcpy(&chunk, p, 8);
                h = _mm_crc32_u64(h, chunk);
                p += 8; len -= 8;
            }
            while (len > 0) {
                h = _mm_crc32_u8((ui32)h, *p++);
                len--;
            }
            hashes[i] = h;
        }
    }
};

struct OJfPrintRow {
    template <typename TCol>
    static inline void Exec(TCol& col, ui64 row, StringVector& out) {
        using T = typename TCol::ElemType;
        out.push_back_mcpy(col.GetData().data() + row, sizeof(T));
    }

    static inline void Exec(TStringColumn& col, ui64 row, StringVector& out) {
        out.push_back_mcpy(
            col.GetData().data() + col.GetData().get_pos(row),
            col.GetData().get_len(row)
        );
    }
};

struct OJfPrintOpt {
    template <typename TCol>
    static inline std::vector<char> Exec(TCol& col) {
        return Serialize(col.GetData());
    }
};

} // namespace JfEngine