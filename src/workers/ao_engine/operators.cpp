#include "operators.h"

#include "columns/operators/dates.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"
#include "columns/operators/filter.h"
#include "columns/operators/vector_like.h"
#include "utils/logger/logger.h"

#include <cassert>

namespace JfEngine {

Expected<void> TPlusOp::ConsumeRowGroup(ITableInput* inp) {
    auto ans_ = args[0]->ThrowRowGroup();

    for (ui64 i = 1; i < args.size(); i++) {
        auto [c, err2] = Do<OAddConst>(ans_, args[i]->GetName());
        ans_ = std::move(c);
    }

    ans = std::move(ans_);

    return EError::NoError;
}

TColumnPtr TPlusOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TPlusOp::Clone() {
    auto r = std::make_unique<TPlusOp>();
    r->is_final = is_final;
    return r;
}

std::string TPlusOp::GetName() const {
    std::string ans = "+(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return ans;
}

TColumnPtr TMinusOp::ThrowRowGroup() {
    return ans;
}

Expected<void> TMinusOp::ConsumeRowGroup(ITableInput* inp) {
    auto ans_ = args[0]->ThrowRowGroup();

    for (ui64 i = 1; i < args.size(); i++) {
        auto [c, err2] = Do<OSubConst>(ans_, args[i]->GetName());
        ans_ = std::move(c);
    }

    ans = std::move(ans_);

    return EError::NoError;
}

std::unique_ptr<IOa> TMinusOp::Clone() {
    auto r = std::make_unique<TMinusOp>();
    r->is_final = is_final;
    return r;
}

std::string TMinusOp::GetName() const {
    std::string ans = "-(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return ans;
}

Expected<void> TLengthOp::ConsumeRowGroup(ITableInput* inp) {
    auto t = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OLength>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

TColumnPtr TLengthOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TLengthOp::Clone() {
    auto r = std::make_unique<TLengthOp>();
    r->is_final = is_final;
    return r;
}

std::string TLengthOp::GetName() const {
    return "LENGTH(" + arg->GetName() + ")";
}

Expected<void> TExtractMinuteOp::ConsumeRowGroup(ITableInput* inp) {
    auto t = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OExtractMinute>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

TColumnPtr TExtractMinuteOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TExtractMinuteOp::Clone() {
    auto r = std::make_unique<TExtractMinuteOp>();
    r->is_final = is_final;
    return r;
}

std::string TExtractMinuteOp::GetName() const {
    return "EXTRACT_MINUTE(" + arg->GetName() + ")";
}

Expected<void> TTruncMinuteOp::ConsumeRowGroup(ITableInput* inp) {
    auto t = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OTruncMinute>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

TColumnPtr TTruncMinuteOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TTruncMinuteOp::Clone() {
    auto r = std::make_unique<TTruncMinuteOp>();
    r->is_final = is_final;
    return r;
}

std::string TTruncMinuteOp::GetName() const {
    return "TRUNC_MINUTE(" + arg->GetName() + ")";
}

Expected<void> TConstIntOp::ConsumeRowGroup(ITableInput* inp) {
    if (!ans) {
        auto t = arg->GetName();

        auto [tans, err] = MakeColumn(std::vector<std::string>{std::move(t)}, EColumn::ki64Column);

        ans = std::move(tans);

        return err;
    }
    return EError::NoError;
}

TColumnPtr TConstIntOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TConstIntOp::Clone() {
    auto r = std::make_unique<TConstIntOp>();
    r->is_final = is_final;
    return r;
}

std::string TConstIntOp::GetName() const {
    return "CONST_INT(" + arg->GetName() + ")";
}

Expected<void> TConstStrOp::ConsumeRowGroup(ITableInput* inp) {
    return EError::NoError;
}

TColumnPtr TConstStrOp::ThrowRowGroup() {
    return nullptr;
}

std::unique_ptr<IOa> TConstStrOp::Clone() {
    auto r = std::make_unique<TConstIntOp>();
    r->is_final = is_final;
    return r;
}

std::string TConstStrOp::GetName() const {
    return arg->GetName();
}

TColumnOp::TColumnOp(std::string name_) :
    name(std::move(name_))
{}

Expected<void> TColumnOp::ConsumeRowGroup(ITableInput* inp) {
    auto [ans_, err] = inp->ReadColumn(name);
    ans = ans_;
    return err;
}

TColumnPtr TColumnOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TColumnOp::Clone() {
    auto r = std::make_unique<TColumnOp>(name);
    r->is_final = is_final;
    return r;
}

std::string TColumnOp::GetName() const {
    return name;
}

Expected<void> TDistinctOp::ConsumeRowGroup(ITableInput* inp) {
    auto col = arg->ThrowRowGroup();
    ans = Do<ODistinctStreamV>(col, cur_sets).GetRes();
    return EError::NoError;
}

TColumnPtr TDistinctOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TDistinctOp::Clone() {
    auto r = std::make_unique<TDistinctOp>();
    r->is_final = is_final;
    return r;
}

std::string TDistinctOp::GetName() const {
    return "DISTINCT(" + arg->GetName() + ")";
}

Expected<void> TRegexpReplaceOp::ConsumeRowGroup(ITableInput* inp) {
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

TColumnPtr TRegexpReplaceOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TRegexpReplaceOp::Clone() {
    auto r = std::make_unique<TRegexpReplaceOp>();
    r->is_final = is_final;
    return r;
}

std::string TRegexpReplaceOp::GetName() const {
    return "REGEXP_REPLACE(" + arg[0]->GetName() + ", '" + arg1_p + "', '" + arg2_p + "')";
}

Expected<void> TIfOp::ConsumeRowGroup(ITableInput* inp) {
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
        JF_LOG(this, "filter col=" << f.column_name
            << " val=" << f.value
            << " col_size=" << col->GetSize()
            << " mask_size=" << res.size()
            << " mask_set=" << res.count());
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
    JF_LOG(this, "then_col_size=" << then_col->GetSize()
        << " els='" << els << "' els_len=" << els.size()
        << " mask_size=" << mask.size()
        << " mask_set=" << mask.count());

    auto [tans, err] = Do<OIfElse>(then_col, els, mask);
    if (err != EError::NoError) {
        return err;
    }

    JF_LOG(this, "out_size=" << (tans ? tans->GetSize() : 0)
        << " ret_err=" << ret_err);

    ans = std::move(tans);

    return ret_err;
}

TColumnPtr TIfOp::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TIfOp::Clone() {
    auto r = std::make_unique<TIfOp>();
    r->cond = cond;
    r->is_final = is_final;
    return r;
}

std::string TIfOp::GetName() const {
    return "IF";
}

} // namespace JfEngine
