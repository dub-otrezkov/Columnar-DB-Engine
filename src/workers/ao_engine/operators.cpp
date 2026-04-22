#include "operators.h"

#include "columns/operators/dates.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"

#include <cassert>

namespace JfEngine {

Expected<void> TPlusOp::ConsumeRowGroup(ITableInput* inp) {
    auto [ans_, _] = args[0]->ThrowRowGroup();


    for (ui64 i = 1; i < args.size(); i++) {
        auto [c, err2] = Do<OAddConst>(ans_, args[i]->GetName());
        ans_ = std::move(c);
    }

    ans = ans_;

    return EError::NoError;
}

Expected<IColumn> TPlusOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TPlusOp::Clone() {
    auto r = std::make_shared<TPlusOp>();
    r->is_final = is_final;
    return std::move(r);
}

std::string TPlusOp::GetName() const {
    std::string ans = "+(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return std::move(ans);
}

Expected<IColumn> TMinusOp::ThrowRowGroup() {
    return ans;
}

Expected<void> TMinusOp::ConsumeRowGroup(ITableInput* inp) {
    auto [ans_, _] = args[0]->ThrowRowGroup();


    for (ui64 i = 1; i < args.size(); i++) {
        auto [c, err2] = Do<OSubConst>(ans_, args[i]->GetName());
        ans_ = std::move(c);
    }

    ans = ans_;

    return EError::NoError;
}

std::shared_ptr<IOa> TMinusOp::Clone() {
    auto r = std::make_shared<TMinusOp>();
    r->is_final = is_final;
    return std::move(r);
}

std::string TMinusOp::GetName() const {
    std::string ans = "-(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return std::move(ans);
}

Expected<void> TLengthOp::ConsumeRowGroup(ITableInput* inp) {
    auto [t, _] = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OLength>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

Expected<IColumn> TLengthOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TLengthOp::Clone() {
    auto r = std::make_shared<TLengthOp>();
    r->is_final = is_final;
    return std::move(r);
}

std::string TLengthOp::GetName() const {
    return "LENGTH(" + arg->GetName() + ")";
}

Expected<void> TExtractMinuteOp::ConsumeRowGroup(ITableInput* inp) {
    auto [t, _] = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OExtractMinute>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return EError::NoError;
}

Expected<IColumn> TExtractMinuteOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TExtractMinuteOp::Clone() {
    auto r = std::make_shared<TExtractMinuteOp>();
    r->is_final = is_final;
    return std::move(r);
}

std::string TExtractMinuteOp::GetName() const {
    return "EXTRACT_MINUTE(" + arg->GetName() + ")";
}

TColumnOp::TColumnOp(std::string name_) :
    name(std::move(name_))
{}

Expected<void> TColumnOp::ConsumeRowGroup(ITableInput* inp) {
    auto [ans_, err] = inp->ReadColumn(name);
    ans = ans_;
    return err;
}

Expected<IColumn> TColumnOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TColumnOp::Clone() {
    auto r = std::make_shared<TColumnOp>(name);
    r->is_final = is_final;
    return std::move(r);
}

std::string TColumnOp::GetName() const {
    return name;
}

Expected<void> TDistinctOp::ConsumeRowGroup(ITableInput* inp) {
    auto [col, _] = arg->ThrowRowGroup();
    ans = Do<ODistinctStreamV>(col, cur_sets).GetShared();
    return EError::NoError;
}

Expected<IColumn> TDistinctOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TDistinctOp::Clone() {
    auto r = std::make_shared<TDistinctOp>();
    r->is_final = is_final;
    return std::move(r);
}

std::string TDistinctOp::GetName() const {
    return "DISTINCT(" + arg->GetName() + ")";
}

} // namespace JfEngine
