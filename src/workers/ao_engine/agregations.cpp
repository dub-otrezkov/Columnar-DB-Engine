#include "agregations.h"

#include "columns/operators/operators.h"
#include "columns/operators/min_max.h"
#include "columns/operators/distinct.h"

#include <cassert>

namespace JfEngine {

Expected<void> TSumAgr::ConsumeRowGroup(ITableInput* inp) {
    auto [col, _] = arg->ThrowRowGroup();

    auto sum = Do<OSum>(col);

    if (sum.HasError()) {
        std::cout << "summ err: " << sum.GetError() << std::endl;
        return sum.GetError();
    }

    if (!ans) {
        ans = sum.GetShared();
    } else {
        auto tmp = Do<OVerticalSum>(ans, sum.GetShared());

        if (tmp.HasError()) {
            return tmp.GetError();
        }

        ans = tmp.GetShared();
    }

    return EError::NoError;
}

Expected<IColumn> TSumAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TSumAgr::Clone() {
    auto r = std::allocate_shared<TSumAgr>(ArenaAlloc());
    r->is_final = is_final;
    return r;
}

std::string TSumAgr::GetName() const {
    return "SUM(" + arg->GetName() + ")";
}

Expected<void> TCountAgr::ConsumeRowGroup(ITableInput* inp) {
    auto [t, _] = arg->ThrowRowGroup();

    ans += t->GetSize();

    return EError::NoError;
}

Expected<IColumn> TCountAgr::ThrowRowGroup() {
    return std::allocate_shared<Ti64Column>(ArenaAlloc(), std::vector<i64>{ans});
}

std::shared_ptr<IOa> TCountAgr::Clone() {
    auto r = std::allocate_shared<TCountAgr>(ArenaAlloc());
    r->is_final = is_final;
    return r;
}

std::string TCountAgr::GetName() const {
    return "COUNT(" + arg->GetName() + ")";
}

Expected<void> TMinAgr::ConsumeRowGroup(ITableInput* inp) {
    auto [col, _] = arg->ThrowRowGroup();

    auto sum = Do<OMin>(col);
    if (sum.HasError()) {
        return sum.GetError();
    }

    if (!ans) {
        ans = sum.GetShared();
    } else {
        auto tmp = Do<OVerticalMin>(ans, sum.GetShared());

        if (tmp.HasError()) {
            return tmp.GetError();
        }

        ans = tmp.GetShared();
    }

    return EError::NoError;
}

Expected<IColumn> TMinAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TMinAgr::Clone() {
    auto r = std::allocate_shared<TMinAgr>(ArenaAlloc());
    r->is_final = is_final;
    return r;
}

std::string TMinAgr::GetName() const {
    return "MIN(" + arg->GetName() + ")";
}

Expected<void> TMaxAgr::ConsumeRowGroup(ITableInput* inp) {
    auto [col, _] = arg->ThrowRowGroup();

    auto sum = Do<OMax>(col);
    if (sum.HasError()) {
        return sum.GetError();
    }

    if (!ans) {
        ans = sum.GetShared();
    } else {
        auto tmp = Do<OVerticalMax>(ans, sum.GetShared());

        if (tmp.HasError()) {
            return tmp.GetError();
        }

        ans = tmp.GetShared();
    }

    return EError::NoError;
}

Expected<IColumn> TMaxAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TMaxAgr::Clone() {
    auto r = std::allocate_shared<TMaxAgr>(ArenaAlloc());
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

Expected<IColumn> TAvgAgr::ThrowRowGroup() {
    i128 avg = 0;
    auto [col, _] = sum.ThrowRowGroup();
    if (col->GetType() == EColumn::ki128Column) {
        avg = static_cast<Ti128Column*>(col.get())->GetData()[0];
    } else if (col->GetType() == EColumn::kDoubleColumn) {
        avg = static_cast<TDoubleColumn*>(col.get())->GetData()[0];
    } else {
        return MakeError<EError::BadArgsErr>("not an int column");
    }

    avg /= static_cast<i128>(static_cast<Ti64Column*>(cnt.ThrowRowGroup().GetShared().get())->GetData()[0]);

    return std::allocate_shared<Ti128Column>(ArenaAlloc(), std::vector<i128>{avg});
}

std::shared_ptr<IOa> TAvgAgr::Clone() {
    auto r = std::allocate_shared<TAvgAgr>(ArenaAlloc());
    r->is_final = is_final;
    return r;
}

std::string TAvgAgr::GetName() const {
    return "AVG(" + arg->GetName() + ")";
}

} // namespace JfEngine