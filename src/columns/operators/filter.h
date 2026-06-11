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

template <typename T> 
static T ParseArg(const std::string& value);

template <std::integral T>
T ParseArg(const std::string& value) {
    if constexpr (sizeof(T) <= 4) {
        return static_cast<T>(std::stoi(value));
    } else {
        return static_cast<T>(std::stoll(value));
    }
}

template <>
ld ParseArg<ld>(const std::string& value) {
    return std::stold(value);
}

template <>
TDate ParseArg<TDate>(const std::string& value) {
    return DateFromStr(value);
}

template <>
TTimestamp ParseArg<TTimestamp>(const std::string& value) {
    return TimestampFromStr(value);
}

struct OFilterShouldSkipBatch {

    enum class EResult {
        kSkipAll,
        kTakeAll,
        kNeedCheck,
    };

    template<CHasMinMax TCol>
    static inline EResult Exec(TCol& col, EFilterType op, const std::string& value) {
        assert(col.GetSize() == 2);
        using T = typename TCol::ElemType;
        T target = ParseArg<T>(value);

        switch (op) {
        case EFilterType::kEq:
            if (target < col.GetData().at(0) || target > col.GetData().at(1)) {
                return EResult::kSkipAll;
            }
            return EResult::kNeedCheck;
        case EFilterType::kNeq:
            if (target < col.GetData().at(0) || target > col.GetData().at(1)) {
                return EResult::kTakeAll;
            }
            return EResult::kNeedCheck;
        case EFilterType::kLess:
            if (target <= col.GetData().at(0)) {
                return EResult::kSkipAll;
            }
            if (target > col.GetData().at(1)) {
                return EResult::kTakeAll;
            }
            return EResult::kNeedCheck;
        case EFilterType::kLeq:
            if (target < col.GetData().at(0)) {
                return EResult::kSkipAll;
            }
            if (target >= col.GetData().at(1)) {
                return EResult::kTakeAll;
            }
            return EResult::kNeedCheck;
        case EFilterType::kGreater:
            if (target < col.GetData().at(0)) {
                return EResult::kTakeAll;
            }
            if (target >= col.GetData().at(1)) {
                return EResult::kSkipAll;
            }
            return EResult::kNeedCheck;
        case EFilterType::kGeq:
            if (target <= col.GetData().at(0)) {
                return EResult::kTakeAll;
            }
            if (target > col.GetData().at(1)) {
                return EResult::kSkipAll;
            }
            return EResult::kNeedCheck;
        default:
            break;
        }
        return EResult::kNeedCheck;
    }

    template<typename TCol>
    static inline EResult Exec(TCol& col, EFilterType op, const std::string& value) {
        return EResult::kNeedCheck;
    }
};

struct OAndCheck {
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
    static inline Expected<void> ExecInner(
        TCol& col, EFilterType op, const std::string& value, boost::dynamic_bitset<>& mask, bool inv) {
        using T = typename TCol::ElemType;
        if (col.GetSize() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        if (op == EFilterType::kLike) {
            return MakeError<EError::UnsupportedErr>();
        }
        T target;
        try {
            target = ParseArg<T>(value);
        } catch (...) {
            return MakeError<EError::NotAnIntErr>();
        }
        auto& data = col.GetData();
        const ui64 n = col.GetSize();
        switch (op) {
            case EFilterType::kEq: {
                for (ui64 i = 0; i < n; i++) {
                    if (!mask[i]) {
                        continue;
                    }
                    if ((data[i] == target) == inv) {
                        mask.reset(i);
                    }
                }
                break;
            }
            case EFilterType::kLess: {
                for (ui64 i = 0; i < n; i++) {
                    if (!mask[i]) {
                        continue;
                    }
                    if ((data[i] < target) == inv) {
                        mask.reset(i);
                    }
                }
                break;
            }
            case EFilterType::kLeq: {
                for (ui64 i = 0; i < n; i++) {
                    if (!mask[i]) {
                        continue;
                    }
                    if ((data[i] <= target) == inv) {
                        mask.reset(i);
                    }
                }
                break;
            }
            default: {
                return MakeError<EError::UnimplementedErr>();
            }
        }
        return EError::NoError;
    }

    static inline Expected<void> ExecInner(
        TStringColumn& col, EFilterType op, const std::string& value,
        boost::dynamic_bitset<>& mask, bool inv) {
        if (col.GetSize() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        auto& data = col.GetData();
        const ui64 n = col.GetSize();
        switch (op) {
            case EFilterType::kEq: {
                if (value.empty()) {
                    for (ui64 i = 0; i < n; i++) {
                        if (!mask[i]) {
                            continue;
                        }
                        if ((data[i].size() == 0) == inv) {
                            mask.reset(i);
                        }
                    }
                } else {
                    for (ui64 i = 0; i < n; i++) {
                        if (!mask[i]) {
                            continue;
                        }
                        if ((data[i] == value) == inv) {
                            mask.reset(i);
                        }
                    }
                }
                break;
            }
            case EFilterType::kLess: {
                for (ui64 i = 0; i < n; i++) {
                    if (!mask[i]) {
                        continue;
                    }
                    if ((data[i] < value) == inv) {
                        mask.reset(i);
                    }
                }
                break;
            }
            case EFilterType::kLeq: {
                for (ui64 i = 0; i < n; i++) {
                    if (!mask[i]) {
                        continue;
                    }
                    if ((data[i] <= value) == inv) {
                        mask.reset(i);
                    }
                }
                break;
            }
            case EFilterType::kLike: {
                // boost::unordered_flat_map<JString, bool> hist;
                if (std::count(value.begin(), value.end(), '%') == 2 && value.at(0) == '%'
                        && value.at(value.size() - 1) == '%') {
                    std::string_view needle(value.data() + 1, value.size() - 2);
                    for (ui64 i = 0; i < n; i++) {
                        if (!mask[i]) {
                            continue;
                        }
                        if (OLikeChecker::Exec(data[i], needle) == inv) {
                            mask.reset(i);
                        }
                    }
                } else {
                    for (ui64 i = 0; i < n; i++) {
                        if (!mask[i]) {
                            continue;
                        }
                        if (LikeMatch(data[i], value) == inv) {
                            mask.reset(i);
                        }
                    }
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
    static constexpr bool kNoDispatch = true;
    static inline Expected<TColumnPtr> Exec(IColumn& col, const boost::dynamic_bitset<>& mask) {
        if (col.GetSize() != mask.size()) {
            return MakeError<EError::BadArgsErr>();
        }
        auto res = MakeEmptyColumn(col.GetType()).GetRes();
        res->ReserveAs(&col, mask.count());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            if (mask[i]) {
                res->Append(&col, i);
            }
        }
        return res;
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
