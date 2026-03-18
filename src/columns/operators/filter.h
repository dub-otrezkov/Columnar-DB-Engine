#include "operators.h"

namespace JFEngine {

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
    static Expected<std::vector<bool>> Exec(T& col, EFilterType op, const std::string& value) {
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

    static Expected<std::vector<bool>> ExecInner(Ti8Column& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        i8 target = 0;
        try {
            target = static_cast<i8>(std::stoi(value));
        } catch (...) {
            std::cout << "not an int" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for ints" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(Ti16Column& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        i16 target = 0;
        try {
            target = static_cast<i16>(std::stoi(value));
        } catch (...) {
            std::cout << "not an int" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for ints" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(Ti32Column& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        i32 target = 0;
        try {
            target = std::stoi(value);
        } catch (...) {
            std::cout << "not an int" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for ints" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(Ti64Column& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        i64 target = 0;
        try {
            target = std::stoll(value);
        } catch (...) {
            std::cout << "not an int" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for ints" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(TDoubleColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        ui64 target = 0;
        try {
            target = std::stold(value);
        } catch (...) {
            std::cout << "not an double" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for ints" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(TDateColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        i64 target = 0;
        try {
            target = DateFromStr(value).IntDate();
        } catch (...) {
            std::cout << "not an dates" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for dates" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i].IntDate() == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i].IntDate() < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i].IntDate() <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(TTimestampColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        i64 target = 0;
        try {
            target = TimestampFromStr(value).IntTime();
        } catch (...) {
            std::cout << "not an dates" << std::endl;
            return MakeError<EError::NotAnIntErr>();
        }
        if (op == EFilterType::kLike) {
            std::cout << "no like for dates" << std::endl;
            return MakeError<EError::UnsupportedErr>();
        }
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i].IntTime() == target));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i].IntTime() < target));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i].IntTime() <= target));
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }

    static Expected<std::vector<bool>> ExecInner(TStringColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        for (ui64 i = 0; i < col.GetSize(); i++) {
            switch (op) {
                case EFilterType::kEq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] == value));
                    break;
                }
                case EFilterType::kLess: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] < value));
                    break;
                }
                case EFilterType::kLeq: {
                    ans[i] = (ans[i] ^ (col.GetData()[i] <= value));
                    break;
                }
                case EFilterType::kLike: {

                    if (value.empty()) {
                        ans[i] = ans[i] ^ 1;
                        break;
                    }
                    if (value == "%") {
                        ans[i] = ans[i] ^ 1;
                        break;
                    }
                    std::string& s = col.GetData()[i];
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
                    break;
                }
                default:
                    std::cout << "not supported op" << std::endl;
                    return MakeError<EError::UnsupportedErr>();
            }
        }
        return ans;
    }
};

struct OFilter {
    static Expected<IColumn> Exec(Ti8Column& col, const std::vector<bool>& mask) {
        std::vector<i8> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<Ti8Column>(std::move(vals));
    }

    static Expected<IColumn> Exec(Ti16Column& col, const std::vector<bool>& mask) {
        std::vector<i16> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<Ti16Column>(std::move(vals));
    }

    static Expected<IColumn> Exec(Ti32Column& col, const std::vector<bool>& mask) {
        std::vector<i32> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<Ti32Column>(std::move(vals));
    }

    static Expected<IColumn> Exec(Ti64Column& col, const std::vector<bool>& mask) {
        std::vector<i64> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<Ti64Column>(std::move(vals));
    }

    static Expected<IColumn> Exec(TDoubleColumn& col, const std::vector<bool>& mask) {
        std::vector<ld> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<TDoubleColumn>(std::move(vals));
    }

    static Expected<IColumn> Exec(TDateColumn& col, const std::vector<bool>& mask) {
        std::vector<TDate> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<TDateColumn>(std::move(vals));
    }

    static Expected<IColumn> Exec(TTimestampColumn& col, const std::vector<bool>& mask) {
        std::vector<TTimestamp> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<TTimestampColumn>(std::move(vals));
    }

    static Expected<IColumn> Exec(TStringColumn& col, const std::vector<bool>& mask) {
        std::vector<std::string> vals;
        ui64 sz = 0;
        for (ui64 i = 0; i < mask.size(); i++) {
            sz += mask[i];
        }
        if (col.GetData().size() != mask.size()) {
            std::cout << "bad filter size" << std::endl;
            return MakeError<EError::BadArgsErr>();
        }
        vals.reserve(sz);
        for (ui64 i = 0; i < col.GetData().size(); i++) {
            if (mask[i]) {
                vals.push_back(col.GetData()[i]);
            }
        }
        return std::make_shared<TStringColumn>(std::move(vals));
    }
};

} // namespace JFEngine