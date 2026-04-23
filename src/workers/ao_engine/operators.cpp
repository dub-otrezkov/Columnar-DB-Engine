#include "operators.h"

#include "columns/operators/dates.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"

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

} // namespace JfEngine
