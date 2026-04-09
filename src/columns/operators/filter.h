#include "operators.h"

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

struct OFilterCheck {
    template<typename T>
    static inline Expected<std::vector<bool>> Exec(T& col, EFilterType op, const std::string& value) {
        switch (op) {
            case EFilterType::kEq: {
                return ExecInner(col, EFilterType::kEq, value);
            }
            case EFilterType::kNeq: {
                return ExecInner(col, EFilterType::kEq, value, /*inv=*/true);
            }
            case EFilterType::kLess: {
                return ExecInner(col, EFilterType::kLess, value);
            }
            case EFilterType::kLeq: {
                return ExecInner(col, EFilterType::kLeq, value);
            }
            case EFilterType::kGreater: {
                return ExecInner(col, EFilterType::kLeq, value, /*inv=*/true);
            }
            case EFilterType::kGeq: {
                return ExecInner(col, EFilterType::kLess, value, /*inv=*/true);
            }
            case EFilterType::kLike: {
                return ExecInner(col, EFilterType::kLike, value);
            }
            case EFilterType::kNLike: {
                return ExecInner(col, EFilterType::kLike, value, /*inv=*/true);
            }
            default:
                return MakeError<EError::UnimplementedErr>();
        }
    }

    template <typename TCol>
    static inline Expected<std::vector<bool>> ExecInner(TCol& col, EFilterType op, const std::string& value, bool inv = false) {
        using T = typename TCol::ElemType;
        std::vector<bool> ans(col.GetSize(), inv);
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
            std::cout << "not an int" << " " << value << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for ints" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        switch (op) {
            case EFilterType::kEq: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == target));
                }
                break;
            }
            case EFilterType::kLess: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < target));
                }
                break;
            }
            case EFilterType::kLeq: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= target));
                }
                break;
            }
        }
        return ans;
    }

    static inline Expected<std::vector<bool>> ExecInner(TStringColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        switch (op) {
            case EFilterType::kEq: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == value));
                }
                break;
            }
            case EFilterType::kLess: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < value));
                }
                break;
            }
            case EFilterType::kLeq: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= value));
                }
                break;
            }
            case EFilterType::kLike: {
                for (ui64 i = 0; i < col.GetSize(); i++) {
                    if (value.empty()) {
                        ans[i] = ans[i] ^ 1;
                        continue;
                    }
                    if (value == "%") {
                        ans[i] = ans[i] ^ 1;
                        continue;
                    }
                    auto s = col.GetData()[i];
                    std::vector<i64> pf(value.size(), 0);
                    i64 st = 0;

                    static auto eq = [](char a, char b) -> bool {
                        return (a == b || a == '_' || b == '_');
                    };

                    auto recalc_pf = [&pf, &st, &value]() -> void {
                        while (st < value.size() && value[st] == '%') {
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
                        if (st == value.size()) {
                            break;
                        }
                        while (curpf > 0 && !eq(s[k], value[st + curpf])) {
                            curpf = pf[st + curpf - 1];
                        }
                        if (eq(s[k], value[st + curpf])) {
                            curpf++;
                        }

                        if (st + curpf == value.size() || value[st + curpf] == '%') {
                            st = st + curpf;
                            curpf = 0;
                            if (st < value.size()) {
                                recalc_pf();
                            }
                        }
                    }


                    bool ans_c = 1;

                    if (st != value.size()) {
                        ans_c = 0;
                    }

                    if (value[0] != '%') {
                        for (ui64 k = 0; k < value.size(); k++) {
                            if (value[k] == '%') {
                                break;
                            }
                            if (!eq(value[k], s[k])) {
                                ans_c = 0;
                                break;
                            }
                        }
                    }

                    if (value[0] != '%') {
                        for (ui64 k = 0; k < value.size(); k++) {
                            if (value[k] == '%') {
                                break;
                            }
                            if (!eq(value[k], s[k])) {
                                ans_c = 0;
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
                                ans_c = 0;
                            }
                        }
                    }

                    ans[i] = (ans[i] ^ ans_c);
                }
                break;
            }
            default:
                std::cout << "not supported op" << std::endl;
                return MakeError<EError::UnsupportedErr>();
        }
        return ans;
    }
};

struct OFilter {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col, const std::vector<bool>& mask) {
        using T = typename TCol::ElemTypeRo;
        std::vector<T> vals;
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<TCol>(std::move(vals));
    }
};

} // namespace JfEngine