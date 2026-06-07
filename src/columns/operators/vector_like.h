#pragma once

#include "../types/types.h"
#include "operators.h"

#include <boost/regex.hpp>

namespace JfEngine {

struct OSetColumnFrom {
    template <typename TCol>
    static inline Expected<void> Exec(TCol& col, TColumnPtr& ans, std::vector<ui64>* idx) {
        if (!ans) {
            ans = MakeEmptyColumn(col.GetType()).GetRes();
        }
        if (col.GetType() != ans->GetType()) {
            return MakeError<EError::BadArgsErr>("types mismatch");
        }
        auto& v = static_cast<TCol*>(ans.get())->GetData();
        auto& id = *idx;
        for (ui64 i = 0; i < id.size(); i++) {
            assert(id.at(i) <= v.size());
            if (id.at(i) == v.size()) {
                v.emplace_back(col.GetData().at(i));
            } else {
                v.at(id.at(i)) = col.GetData().at(i);
            }
        }

        return EError::NoError;
    }
};

struct OOffset {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, i64 offset) {
        using T = typename TCol::ElemType;
        i64 safe_offset = std::min(offset, static_cast<i64>(col.GetData().size()));
        std::vector<T> ans(col.GetData().size() - safe_offset);
        std::memcpy(
            reinterpret_cast<char*>(ans.data()),
            reinterpret_cast<char*>(col.GetData().data() + safe_offset),
            ans.size() * sizeof(T)
        );
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
        vals.reserve(col.GetData().size());
        
        boost::regex re(arg1);
        std::string res;
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            auto& t = col.GetData().at(i);
            res.clear();
            boost::regex_replace(std::back_inserter(res), t.begin(), t.end(), re, arg2);
            vals.emplace_back(res.size(), res.data());
        }
        return std::make_shared<TStringColumn>(std::move(vals));
    }
};

} // namespace JfEngine
