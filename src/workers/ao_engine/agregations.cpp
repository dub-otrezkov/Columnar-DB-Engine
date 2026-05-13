#include "agregations.h"

#include "columns/operators/distinct.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"

#include <cassert>

namespace JfEngine {

Expected<void> TSumAgr::ConsumeRowGroup(ITableInput*, ui64 idx) {
    auto v = Do<OSum>(arg->ThrowRowGroup());
    if (v.HasError()) return v.GetError();
    CombineAt<OAddAtIdx>(idx, v.GetRes());
    return EError::NoError;
}

Expected<void> TMinAgr::ConsumeRowGroup(ITableInput*, ui64 idx) {
    auto v = Do<OMin>(arg->ThrowRowGroup());
    if (v.HasError()) return v.GetError();
    CombineAt<OMinAtIdx>(idx, v.GetRes());
    return EError::NoError;
}

Expected<void> TMaxAgr::ConsumeRowGroup(ITableInput*, ui64 idx) {
    auto v = Do<OMax>(arg->ThrowRowGroup());
    if (v.HasError()) return v.GetError();
    CombineAt<OMaxAtIdx>(idx, v.GetRes());
    return EError::NoError;
}

Expected<void> TCountAgr::ConsumeRowGroup(ITableInput*, ui64 idx) {
    if (!ans) {
        ans = std::make_shared<Ti64Column>(std::vector<i64>(used.size(), 0));
    }
    static_cast<Ti64Column*>(ans.get())->GetData().at(idx) += arg->ThrowRowGroup()->GetSize();
    used[idx] = true;
    return EError::NoError;
}

Expected<void> TAvgAgr::ConsumeRowGroup(ITableInput* inp, ui64 idx) {
    if (count.size() < idx + 1) {
        count.resize(idx + 1, 0);
    }
    count.at(idx) += arg->ThrowRowGroup()->GetSize();
    return EError::NoError;
}

TColumnPtr TAvgAgr::ThrowRowGroup() {
    if (!ans) {
        ans = Do<OMakeAvg>(sum.ThrowRowGroup(), count).GetRes();
    }
    return ans;
}

std::string TSumAgr::GetName() const {
    return "SUM(" + arg->GetName() + ")";
}

std::string TCountAgr::GetName() const {
    return "COUNT(" + arg->GetName() + ")";
}

std::string TMinAgr::GetName() const {
    return "MIN(" + arg->GetName() + ")";
}

std::string TMaxAgr::GetName() const {
    return "MAX(" + arg->GetName() + ")";
}

std::string TAvgAgr::GetName() const {
    return "AVG(" + arg->GetName() + ")";
}

} // namespace JfEngine