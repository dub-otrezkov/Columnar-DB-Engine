#pragma once

#include "operators.h"

#include "utils/errors/errors.h"

#include "utils/like/combined.h"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace JfEngine {

enum EFilterType {
    kEq,
    kLeq,
    kLess,
    kGreater,
    kGeq,
    kNeq,
    kIn,
    kNIn,
    kLike,
    kNLike
};

struct OAndCheck {
    template <typename TCol, typename Pred>
    static inline void ApplyMask(TCol& col, Pred pred, boost::dynamic_bitset<>& mask, bool inv) {
        auto& data = col.GetData();
        const ui64 n = mask.size();
        for (ui64 i = 0; i < n; i++) {
            if (!mask[i]) {
                continue;
            }
            if (!(pred(data[i]) ^ inv)) {
                mask.reset(i);
            }
        }
    }

    template<typename T>
    static inline Expected<void> Exec(T& col, EFilterType op, const std::string& value, boost::dynamic_bitset<>& mask) {
        switch (op) {
            case EFilterType::kEq: {
                return ExecInner(col, EFilterType::kEq, value, mask, /*inv=*/false);
            }
            case EFilterType::kNeq: {
                return ExecInner(col, EFilterType::kEq, value, mask, /*inv=*/true);
            }
            case EFilterType::kLess: {
                return ExecInner(col, EFilterType::kLess, value, mask, /*inv=*/false);
            }
            case EFilterType::kLeq: {
                return ExecInner(col, EFilterType::kLeq, value, mask, /*inv=*/false);
            }
            case EFilterType::kGreater: {
                return ExecInner(col, EFilterType::kLeq, value, mask, /*inv=*/true);
            }
            case EFilterType::kGeq: {
                return ExecInner(col, EFilterType::kLess, value, mask, /*inv=*/true);
            }
            case EFilterType::kLike: {
                return ExecInner(col, EFilterType::kLike, value, mask, /*inv=*/false);
            }
            case EFilterType::kNLike: {
                return ExecInner(col, EFilterType::kLike, value, mask, /*inv=*/true);
            }
            default:
                return MakeError<EError::UnimplementedErr>();
        }
    }

    template <typename TCol>
    static inline Expected<void> ExecInner(TCol& col, EFilterType op, const std::string& value, boost::dynamic_bitset<>& mask, bool inv) {
        using T = typename TCol::ElemType;
        if (col.GetSize() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        if (op == EFilterType::kLike) {
            return MakeError<EError::UnsupportedErr>();
        }
        T target;
        try {
            if constexpr (std::is_same_v<TCol, TDateColumn>) {
                target = DateFromStr(value);
            } else if constexpr (std::is_same_v<TCol, TTimestampColumn>) {
                target = TimestampFromStr(value);
            } else if constexpr (std::is_same_v<TCol, TDoubleColumn>) {
                target = static_cast<T>(std::stold(value));
            } else {
                target = static_cast<T>(std::stoll(value));
            }
        } catch (...) {
            return MakeError<EError::NotAnIntErr>();
        }
        switch (op) {
            case EFilterType::kEq: {
                ApplyMask(col, [&](const T& v) {
                    return v == target;
                }, mask, inv);
                break;
            }
            case EFilterType::kLess: {
                ApplyMask(col, [&](const T& v) {
                    return v < target;
                }, mask, inv);
                break;
            }
            case EFilterType::kLeq: {
                ApplyMask(col, [&](const T& v) {
                    return v <= target;
                }, mask, inv);
                break;
            }
            default: {
                return MakeError<EError::UnimplementedErr>();
            }
        }
        return EError::NoError;
    }

    static inline Expected<void> ExecInner(TStringColumn& col, EFilterType op, const std::string& value, boost::dynamic_bitset<>& mask, bool inv) {
        if (col.GetSize() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        switch (op) {
            case EFilterType::kEq: {
                if (value.empty()) {
                    ApplyMask(col, [](const JString& s) {
                        return s.size() == 0;
                    }, mask, inv);
                } else {
                    ApplyMask(col, [&](const JString& s) {
                        return s == value;
                    }, mask, inv);
                }
                break;
            }
            case EFilterType::kLess: {
                ApplyMask(col, [&](const JString& s) {
                    return s < value;
                }, mask, inv);
                break;
            }
            case EFilterType::kLeq: {
                ApplyMask(col, [&](const JString& s) {
                    return s <= value;
                }, mask, inv);
                break;
            }
            case EFilterType::kLike: {
                if (std::count(value.begin(), value.end(), '%') == 2 && value.at(0) == '%' && value.at(value.size() - 1) == '%') {
                    std::string_view needle(value.data() + 1, value.size() - 2);
                    ApplyMask(col, [&](JString& s) {
                        return OLikeChecker::Exec(s, needle);
                    }, mask, inv);
                } else {
                    ApplyMask(col, [&](JString& s) {
                        return LikeMatch(s, value);
                    }, mask, inv);
                }
                break;
            }
            default: {
                return MakeError<EError::UnsupportedErr>();
            }
        }
        return EError::NoError;
    }

    static inline bool LikeMatch(JString& s, const std::string& value) {
        if (value.empty()) {
            return true;
        }
        if (value == "%") {
            return true;
        }
        std::vector<i64> pf(value.size(), 0);
        i64 st = 0;

        static auto eq = [](char a, char b) -> bool {
            return (a == b || a == '_' || b == '_');
        };

        auto recalc_pf = [&pf, &st, &value]() -> void {
            while (st < static_cast<i64>(value.size()) && value[st] == '%') {
                st++;
            }
            for (ui64 t = st + 1; t < value.size() && value[t] != '%'; t++) {
                pf[t] = pf[t - 1];
                while (pf[t] > 0 && !eq(value[pf[t] + st], value[t])) {
                    pf[t] = pf[pf[t] + st - 1];
                }
                if (eq(value[t], value[st + pf[t]])) {
                    pf[t]++;
                }
            }
        };
        recalc_pf();
        ui64 curpf = 0;
        for (ui64 k = 0; k < s.size(); k++) {
            if (st == static_cast<i64>(value.size())) {
                break;
            }
            while (curpf > 0 && !eq(s[k], value[st + curpf])) {
                curpf = pf[st + curpf - 1];
            }
            if (eq(s[k], value[st + curpf])) {
                curpf++;
            }
            if (st + curpf == static_cast<i64>(value.size()) || value[st + curpf] == '%') {
                st = st + curpf;
                curpf = 0;
                if (st < static_cast<i64>(value.size())) {
                    recalc_pf();
                }
            }
        }

        bool ans_c = true;
        if (st != static_cast<i64>(value.size())) {
            ans_c = false;
        }
        if (value[0] != '%') {
            for (ui64 k = 0; k < value.size(); k++) {
                if (value[k] == '%') {
                    break;
                }
                if (!eq(value[k], s[k])) {
                    ans_c = false;
                    break;
                }
            }
        }
        if (value.back() != '%') {
            for (ui64 k = 1; k <= value.size(); k++) {
                if (value[value.size() - k] == '%') {
                    break;
                }
                if (!eq(value[value.size() - k], s[s.size() - k])) {
                    ans_c = false;
                }
            }
        }
        return ans_c;
    }
};

struct OFilter {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, const boost::dynamic_bitset<>& mask) {
        using T = typename TCol::ElemTypeRo;
        std::vector<T> vals;
        if (col.GetData().size() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<TCol>(std::move(vals));
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col, const boost::dynamic_bitset<>& mask) {
        std::vector<JString> vals;
        if (col.GetData().size() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData().at(i));
            }
        }
        return std::make_shared<TStringColumn>(std::move(vals));
    }
};

struct OIfElse {
    template <typename TCol>
    static inline Expected<TColumnPtr> Exec(TCol& col, const std::string& els, const boost::dynamic_bitset<>& mask) {
        return MakeError<EError::UnimplementedErr>("im lazyyyy");
    }

    static inline Expected<TColumnPtr> Exec(TStringColumn& col, const std::string& els, const boost::dynamic_bitset<>& mask) {
        std::vector<JString> vals;
        if (col.GetData().size() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.emplace_back(col.GetData().at(i));
            } else {
                vals.emplace_back(std::string_view(els));
            }
        }
        return std::make_shared<TStringColumn>(std::move(vals));
    }
};

} // namespace JfEngine