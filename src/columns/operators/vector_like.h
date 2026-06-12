#pragma once

#include "../types/types.h"
#include "operators.h"

#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>

namespace JfEngine {

struct OPushBack {
    template <typename TCol>
    static inline void Exec(TCol& col, typename TCol::ElemTypeRo value) {
        col.GetData().emplace_back(value);
    }

    static inline void Exec(TStringColumn& col, std::string_view value) {
        col.GetData().emplace_back(value);
    }
};

struct OSetColumnFrom {
    static constexpr bool kNoDispatch = true;
    static inline Expected<void> Exec(IColumn& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        if (!ans) {
            ans = MakeEmptyColumn(col.GetType()).GetRes();
        }
        if (col.GetType() != ans->GetType()) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }
        for (ui64 row = 0; row < idx->size(); row++) {
            ui64 g = (*idx)[row];
            if (g == ans->GetSize()) {
                ans->Append(&col, row);
            }
        }
        return EError::NoError;
    }
};

struct OOffset {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, i64 offset) {
        using T = typename TCol::ElemType;
        auto& data = col.GetData();
        i64 safe_offset = std::min(offset, static_cast<i64>(data.size()));
        std::vector<T> ans(data.begin() + safe_offset, data.end());
        return std::make_shared<TCol>(std::move(ans));
    }
};

struct OCloneConst {
    template <typename TCol>
    static inline Expected<void> Exec(TCol& col, ui64 new_size) {
        auto& data = col.GetData();
        data.assign(new_size, data.at(0));
        return EError::NoError;
    }
};

struct TJStringGetter {
    using FnPtr = JString (*)(char*, ui64);
    char* ctx;
    FnPtr fn;

    JString operator()(ui64 i) const {
        return fn(ctx, i);
    }
};

struct OToRawBytes {
    template <typename TCol>
    static inline std::pair<const char*, ui64> Exec(TCol& col) {
        using T = typename TCol::ElemType;
        return {
            reinterpret_cast<const char*>(col.GetData().data()),
            sizeof(T)
        };
    }

    static inline std::pair<const char*, ui64> Exec(TStringColumn& col) {
        return {nullptr, 0};
    }
};

struct OToJStrings {
    template <typename TCol>
    static inline TJStringGetter Exec(TCol& col) {
        using T = typename TCol::ElemType;
        return TJStringGetter{
            reinterpret_cast<char*>(col.GetData().data()),
            +[](char* p, ui64 i) -> JString {
                return JString{
                    static_cast<ui32>(sizeof(T)),
                    p + i * sizeof(T)
                };
            }
        };
    }

    static inline TJStringGetter Exec(TStringColumn& col) {
        return TJStringGetter{
            reinterpret_cast<char*>(col.GetData().data()),
            +[](char* p, ui64 i) -> JString {
                return *reinterpret_cast<JString*>(p + i * sizeof(JString));
            }
        };
    }
};


struct ORegexpReplace {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, const std::string& arg1, const std::string& arg2) {
        return MakeError<EError::UnimplementedErr>("regexp only 4 str");
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col, const std::string& arg1, const std::string& arg2) {
        std::vector<JString> vals;
        vals.reserve(col.GetSize());

        static boost::unordered_flat_map<
            std::pair<std::string, std::string>,
            boost::unordered_flat_map<JString, JString>
        > gCache;
        auto& stor = gCache[std::make_pair(arg1, arg2)];

        boost::regex re(arg1);
        std::string res;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            auto& t = col.GetData().at(i);
            auto it = stor.find(t);
            if (it != stor.end()) {
                vals.push_back(it->second);
            } else {
                res.clear();
                boost::regex_replace(std::back_inserter(res), t.begin(), t.end(), re, arg2);
                vals.emplace_back(res.size(), res.data());
                stor.emplace(t, vals.back());
            }
        }
        return std::make_shared<TStringColumn>(std::move(vals));
    }
};

} // namespace JfEngine
