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
            default:
                return MakeError<EError::UnimplementedErr>();
        }
    }

    static Expected<std::vector<bool>> ExecInner(Ti8Column& col, EFilterType op, const std::string& value, bool inv = false) {
        return MakeError<EError::UnimplementedErr>();
    }

    static Expected<std::vector<bool>> ExecInner(Ti16Column& col, EFilterType op, const std::string& value, bool inv = false) {
        return MakeError<EError::UnimplementedErr>();
    }

    static Expected<std::vector<bool>> ExecInner(Ti32Column& col, EFilterType op, const std::string& value, bool inv = false) {
        return MakeError<EError::UnimplementedErr>();
    }

    static Expected<std::vector<bool>> ExecInner(Ti64Column& col, EFilterType op, const std::string& value, bool inv = false) {
        std::vector<bool> ans(col.GetSize(), inv);
        ui64 target = 0;
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
        return MakeError<EError::UnimplementedErr>();
    }

    static Expected<std::vector<bool>> ExecInner(TDateColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        return MakeError<EError::UnimplementedErr>();
    }

    static Expected<std::vector<bool>> ExecInner(TTimestampColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        return MakeError<EError::UnimplementedErr>();
    }

    static Expected<std::vector<bool>> ExecInner(TStringColumn& col, EFilterType op, const std::string& value, bool inv = false) {
        return MakeError<EError::UnimplementedErr>();
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