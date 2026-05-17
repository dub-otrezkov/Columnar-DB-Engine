#include "agregations.h"

#include "columns/operators/distinct.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"

#include "utils/logger/logger.h"

#include <cassert>

namespace JfEngine {

Expected<void> TSumAgr::ConsumeRowGroup(ITableInput*, std::vector<ui64>* idx) {
    // JF_LOG(this, "got arg: " << arg << std::endl);
    auto v = arg->ThrowRowGroup();
    return Do<OMultipleAdder>(v, ans, idx);
}

Expected<void> TMinAgr::ConsumeRowGroup(ITableInput*, std::vector<ui64>* idx) {
    auto v = arg->ThrowRowGroup();
    return Do<OMultipleMin>(v, ans, idx);
}

Expected<void> TMaxAgr::ConsumeRowGroup(ITableInput*, std::vector<ui64>* idx) {
    auto v = arg->ThrowRowGroup();
    return Do<OMultipleMax>(v, ans, idx);
}

Expected<void> TCountAgr::ConsumeRowGroup(ITableInput*, std::vector<ui64>* idx) {
    if (!ans) {
        ans = std::make_shared<Ti64Column>(std::vector<i64>(1, 0));
    }
    auto& v = static_cast<Ti64Column*>(ans.get())->GetData();
    auto col = arg->ThrowRowGroup();
    if (!idx) {
        if (v.empty()) {
            v.push_back(0);
        }
        v.at(0) += col->GetSize();
    } else {
        auto& id = *idx;
        for (ui64 i = 0; i < col->GetSize(); i++) {
            assert(v.size() <= id.at(i));
            if (v.size() == id.at(i)) {
                v.push_back(0);
            }
            v.at(id.at(i))++;
        }
    }
    return EError::NoError;
}

Expected<void> TAvgAgr::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) {
    sum.ConsumeRowGroup(inp, idx);
    cnt.ConsumeRowGroup(inp, idx);
    return EError::NoError;
}

Expected<void> TCountDistinctAgr::ConsumeRowGroup(ITableInput*, std::vector<ui64>* idx) {
    if (!ans) {
        ans = std::make_shared<Ti64Column>(std::vector<i64>(1, 0));
    }
    if (!idx) {
        static_cast<Ti64Column*>(ans.get())->GetData().at(0) += Do<ODistinctCountDelta>(
            arg->ThrowRowGroup(),
            cur_sets,
            0
        ).GetRes();
    } else {
        
    }
    return EError::NoError;
}

TColumnPtr TAvgAgr::ThrowRowGroup() {
    if (!ans) {
        ans = Do<OMakeAvg>(sum.ThrowRowGroup(), cnt).GetRes();
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

std::string TCountDistinctAgr::GetName() const {
    return "COUNT_DISTINCT(" + arg->GetName() + ")";
}

} // namespace JfEngine