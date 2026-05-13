#pragma once

#include "../types/types.h"
#include "operators.h"

#include <boost/regex.hpp>

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

struct OPushBackEmpty {
    template <typename TCol>
    static inline void Exec(TCol& col) {
        col.GetData().emplace_back();
    }
};

// from, to, i
struct OPushBackFrom {
    template<typename TCol>
    static inline void Exec(TCol& from, TColumnPtr to, i64 i) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        OPushBack::Exec(*static_cast<TCol*>(to.get()), from.GetData().at(i));
    }
};

struct OPushBackFromBatch {
    template<typename TCol>
    static inline void Exec(TCol& from, TColumnPtr to, std::vector<ui64>& is) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        auto& t = *static_cast<TCol*>(to.get());
        t.GetData().reserve(t.GetData().size() + is.size());
        for (const auto& i : is) {
            assert(i < from.GetData().size());
            OPushBack::Exec(t, from.GetData().at(i));
        }
    }
};

// from, to
struct OPushBackVector {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& from, TColumnPtr to) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        auto target = static_cast<TCol*>(to.get());
        ui64 prev_sz = target->GetData().size();
        target->GetData().resize(from.GetData().size() + target->GetData().size());
        std::memcpy(
            reinterpret_cast<char*>(target->GetData().data() + prev_sz),
            reinterpret_cast<char*>(from.GetData().data()),
            from.GetData().size() * sizeof(typename TCol::ElemType)
        );
        return EError::NoError;
    }
};

struct OResize {
    template <typename T>
    static inline Expected<void> Exec(T& col, i64 len) {
        col.GetData().resize(len);
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

struct OClear {
    template <typename TCol>
    static inline Expected<void> Exec(TCol& col) {
        col.GetData().clear();
        return EError::NoError;
    }
};

struct OToJStrings {
    template <typename TCol>
    static inline std::vector<JString> Exec(TCol& col) {
        auto& data = col.GetData();
        std::vector<JString> ans(data.size());
        for (ui64 i = 0; i < data.size(); i++) {
            ans[i] = JString(data.at(i));
        }
        return ans;
    }

    static inline std::vector<JString> Exec(TStringColumn& col) {
        return col.GetData();
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
            auto t = col.GetData().at(i).to_string();
            res.clear();
            boost::regex_replace(std::back_inserter(res), t.begin(), t.end(), re, arg2);
            vals.emplace_back(std::string_view(res));
        }
        return std::make_shared<TStringColumn>(std::move(vals));
    }
};

} // namespace JfEngine
