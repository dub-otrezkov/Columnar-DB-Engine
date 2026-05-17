#include "operators.h"

#include "columns/operators/dates.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"
#include "columns/operators/filter.h"
#include "columns/operators/vector_like.h"
#include "utils/logger/logger.h"

#include <cassert>

namespace JfEngine {

Expected<void> TPlusOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    auto ans_ = args[0]->ThrowRowGroup();

    for (ui64 i = 1; i < args.size(); i++) {
        auto [c, err2] = Do<OAddConst>(ans_, args[i]->GetName());
        ans_ = std::move(c);
    }

    ans = std::move(ans_);

    return EError::NoError;
}

std::string TPlusOp::GetName() const {
    std::string ans = "+(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return ans;
}

Expected<void> TMinusOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    auto ans_ = args[0]->ThrowRowGroup();

    for (ui64 i = 1; i < args.size(); i++) {
        auto [c, err2] = Do<OSubConst>(ans_, args[i]->GetName());
        ans_ = std::move(c);
    }

    ans = std::move(ans_);

    return EError::NoError;
}

std::string TMinusOp::GetName() const {
    std::string ans = "-(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return ans;
}

Expected<void> TLengthOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    auto t = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OLength>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

std::string TLengthOp::GetName() const {
    return "LENGTH(" + arg->GetName() + ")";
}

Expected<void> TExtractMinuteOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    auto t = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OExtractMinute>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

std::string TExtractMinuteOp::GetName() const {
    return "EXTRACT_MINUTE(" + arg->GetName() + ")";
}

Expected<void> TTruncMinuteOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    auto t = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OTruncMinute>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

std::string TTruncMinuteOp::GetName() const {
    return "TRUNC_MINUTE(" + arg->GetName() + ")";
}

Expected<void> TConstIntOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    if (!ans) {
        auto t = arg->GetName();

        auto [tans, err] = MakeColumn(std::vector<std::string>{std::move(t)}, EColumn::ki64Column);

        ans = std::move(tans);

        return err;
    }
    return EError::NoError;
}

std::string TConstIntOp::GetName() const {
    return "CONST_INT(" + arg->GetName() + ")";
}

Expected<void> TConstStrOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    return EError::NoError;
}

std::string TConstStrOp::GetName() const {
    return arg->GetName();
}

TColumnOp::TColumnOp(std::string name_) :
    name(std::move(name_))
{}

Expected<void> TColumnOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) {
    auto [ans_, err] = inp->ReadColumn(name);
    if (is_group_key && ans_ && ans_->GetSize() > 0) {
        Do<OSetColumnFrom>(ans_, ans, idx);
    } else {
        ans = ans_;
    }
    return err;
}

std::string TColumnOp::GetName() const {
    return name;
}

Expected<void> TRegexpReplaceOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    auto col = arg[0]->ThrowRowGroup();
    if (!col) {
        return MakeError<EError::BadArgsErr>("REGEXP_REPLACE: source column is null");
    }
    auto res = Do<ORegexpReplace>(col, arg1_p, arg2_p);
    if (res.HasError()) {
        return res.GetError();
    }
    ans = res.GetRes();
    return EError::NoError;
}

std::string TRegexpReplaceOp::GetName() const {
    return "REGEXP_REPLACE(" + arg[0]->GetName() + ", '" + arg1_p + "', '" + arg2_p + "')";
}

Expected<void> TIfOp::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>*) {
    boost::dynamic_bitset<> mask;
    EError ret_err = EError::NoError;
    for (auto& f : cond.fils) {
        auto [col, err_read] = inp->ReadColumn(f.column_name);
        if (err_read != EError::NoError) {
            if (err_read == EError::EofErr) {
                ret_err = err_read;
            } else {
                return err_read;
            }
        }
        if (!col) {
            return err_read;
        }
        auto [res, err] = Do<OFilterCheck>(col, f.op, f.value);
        if (err) {
            return err;
        }
        if (mask.empty()) {
            mask = std::move(res);
        } else {
            assert(mask.size() == res.size());
            mask &= res;
        }
    }

    auto then_col = arg[0]->ThrowRowGroup();
    auto els = arg[1]->GetName();
    if (!then_col) {
        return ret_err;
    }

    auto [tans, err] = Do<OIfElse>(then_col, els, mask);
    if (err != EError::NoError) {
        return err;
    }

    ans = std::move(tans);

    return ret_err;
}

std::string TIfOp::GetName() const {
    return "IF";
}

} // namespace JfEngine
