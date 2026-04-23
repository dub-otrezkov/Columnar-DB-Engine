#include "agregations.h"

#include "columns/operators/distinct.h"
#include "columns/operators/min_max.h"
#include "columns/operators/operators.h"

#include <cassert>

namespace JfEngine {

Expected<void> TSumAgr::ConsumeRowGroup(ITableInput* inp) {
    auto col = arg->ThrowRowGroup();

    auto sum = Do<OSum>(col);

    if (sum.HasError()) {
        return sum.GetError();
    }

    if (!ans) {
        ans = sum.GetRes();
    } else {
        auto tmp = Do<OVerticalSum>(ans, sum.GetRes());

        if (tmp.HasError()) {
            return tmp.GetError();
        }

        ans = tmp.GetRes();
    }

    return EError::NoError;
}

TColumnPtr TSumAgr::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TSumAgr::Clone() {
    auto r = std::make_unique<TSumAgr>();
    r->is_final = is_final;
    return r;
}

std::string TSumAgr::GetName() const {
    return "SUM(" + arg->GetName() + ")";
}

Expected<void> TCountAgr::ConsumeRowGroup(ITableInput* inp) {
    auto t = arg->ThrowRowGroup();

    ans += t->GetSize();

    return EError::NoError;
}

TColumnPtr TCountAgr::ThrowRowGroup() {
    return std::make_shared<Ti64Column>(std::vector<i64>{ans});
}

std::unique_ptr<IOa> TCountAgr::Clone() {
    auto r = std::make_unique<TCountAgr>();
    r->is_final = is_final;
    return r;
}

std::string TCountAgr::GetName() const {
    return "COUNT(" + arg->GetName() + ")";
}

Expected<void> TMinAgr::ConsumeRowGroup(ITableInput* inp) {
    auto col = arg->ThrowRowGroup();

    auto sum = Do<OMin>(col);
    if (sum.HasError()) {
        return sum.GetError();
    }

    if (!ans) {
        ans = sum.GetRes();
    } else {
        auto tmp = Do<OVerticalMin>(ans, sum.GetRes());

        if (tmp.HasError()) {
            return tmp.GetError();
        }

        ans = tmp.GetRes();
    }

    return EError::NoError;
}

TColumnPtr TMinAgr::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TMinAgr::Clone() {
    auto r = std::make_unique<TMinAgr>();
    r->is_final = is_final;
    return r;
}

std::string TMinAgr::GetName() const {
    return "MIN(" + arg->GetName() + ")";
}

Expected<void> TMaxAgr::ConsumeRowGroup(ITableInput* inp) {
    auto col = arg->ThrowRowGroup();

    auto sum = Do<OMax>(col);
    if (sum.HasError()) {
        return sum.GetError();
    }

    if (!ans) {
        ans = sum.GetRes();
    } else {
        auto tmp = Do<OVerticalMax>(ans, sum.GetRes());

        if (tmp.HasError()) {
            return tmp.GetError();
        }

        ans = tmp.GetRes();
    }

    return EError::NoError;
}

TColumnPtr TMaxAgr::ThrowRowGroup() {
    return ans;
}

std::unique_ptr<IOa> TMaxAgr::Clone() {
    auto r = std::make_unique<TMaxAgr>();
    r->is_final = is_final;
    return r;
}

std::string TMaxAgr::GetName() const {
    return "MAX(" + arg->GetName() + ")";
}

Expected<void> TAvgAgr::ConsumeRowGroup(ITableInput* inp) {
    bool run = 1;

    i64 len = 0;

    if (!inited) {
        sum.AddArg(arg);
        cnt.AddArg(arg);
        inited = true;
    }

    auto sum_col = sum.ConsumeRowGroup(inp);

    auto cnt_col = cnt.ConsumeRowGroup(inp);

    return EError::NoError;
}

TColumnPtr TAvgAgr::ThrowRowGroup() {
    i128 avg = 0;
    auto col = sum.ThrowRowGroup();
    if (col->GetType() == EColumn::ki128Column) {
        avg = static_cast<Ti128Column*>(col.get())->GetData()[0];
    } else if (col->GetType() == EColumn::kDoubleColumn) {
        avg = static_cast<TDoubleColumn*>(col.get())->GetData()[0];
    } else {
        return nullptr;
    }

    avg /= static_cast<i128>(static_cast<Ti64Column*>(cnt.ThrowRowGroup().get())->GetData()[0]);

    return std::make_shared<Ti128Column>(std::vector<i128>{avg});
}

std::unique_ptr<IOa> TAvgAgr::Clone() {
    auto r = std::make_unique<TAvgAgr>();
    r->is_final = is_final;
    return r;
}

std::string TAvgAgr::GetName() const {
    return "AVG(" + arg->GetName() + ")";
}

} // namespace JfEngine