#include "agregations.h"

#include "columns/operators/operators.h"
#include "columns/operators/min_max.h"
#include "columns/operators/distinct.h"

#include <cassert>

namespace JfEngine {

Expected<void> TSumAgr::ConsumeRowGroup(ITableInput* inp) {
    auto err = arg->ConsumeRowGroup(inp);

    bool is_eof = false;
    if (err.GetError() != EError::NoError) {
        if (EError::EofErr == err.GetError()) {
            is_eof = true;
        } else {
            return err;
        }
    }

    auto [col, _] = arg->ThrowRowGroup();

    auto sum = Do<OSum>(col);
    if (sum.HasError()) {
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

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TSumAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TSumAgr::Clone() {
    auto r = std::allocate_shared<TSumAgr>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TSumAgr::GetName() const {
    return "SUM(" + arg->GetName() + ")";
}

Expected<void> TCountAgr::ConsumeRowGroup(ITableInput* inp) {
    auto err = arg->ConsumeRowGroup(inp);
    bool is_eof = false;
    if (err.HasError()) {
        if (err.GetError() == EError::EofErr) {
            is_eof = true;
        } else {
            return err.GetError();
        }
    }

    auto [t, _] = arg->ThrowRowGroup();

    ans += t->GetSize();

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TCountAgr::ThrowRowGroup() {
    return std::allocate_shared<Ti64Column>(ArenaAlloc(), std::vector<i64>{ans});
}

std::shared_ptr<IOa> TCountAgr::Clone() {
    auto r = std::allocate_shared<TCountAgr>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TCountAgr::GetName() const {
    return "COUNT(" + arg->GetName() + ")";
}

Expected<void> TMinAgr::ConsumeRowGroup(ITableInput* inp) {
    auto err = arg->ConsumeRowGroup(inp);
    bool is_eof = false;
    if (err.GetError() != EError::NoError) {
        if (EError::EofErr == err.GetError()) {
            is_eof = true;
        } else {
            return err;
        }
    }

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

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TMinAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TMinAgr::Clone() {
    auto r = std::allocate_shared<TMinAgr>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TMinAgr::GetName() const {
    return "MIN(" + arg->GetName() + ")";
}

Expected<void> TMaxAgr::ConsumeRowGroup(ITableInput* inp) {
    auto err = arg->ConsumeRowGroup(inp);
    bool is_eof = false;
    if (err.GetError() != EError::NoError) {
        if (EError::EofErr == err.GetError()) {
            is_eof = true;
        } else {
            return err;
        }
    }

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

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TMaxAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TMaxAgr::Clone() {
    auto r = std::allocate_shared<TMaxAgr>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TMaxAgr::GetName() const {
    return "MAX(" + arg->GetName() + ")";
}

Expected<void> TAvgAgr::ConsumeRowGroup(ITableInput* inp) {
    bool run = 1;

    i64 len = 0;

    if (!inited) {
        sum.AddArg(arg->Clone());
        cnt.AddArg(std::move(arg));
        inited = true;
    }

    bool is_eof = false;

    auto sum_col = sum.ConsumeRowGroup(inp);
    if (sum_col.HasError()) {
        if (sum_col.GetError() == EError::EofErr) {
            is_eof = true;
        } else {
            return sum_col.GetError();
        }
    }

    auto cnt_col = cnt.ConsumeRowGroup(inp);
    if (cnt_col.HasError()) {
        if (cnt_col.GetError() == EError::EofErr) {
            is_eof = true;
        } else {
            return cnt_col.GetError();
        }
    }

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TAvgAgr::ThrowRowGroup() {
    ld avg = 0;
    auto [col, _] = sum.ThrowRowGroup();
    if (col->GetType() == EColumn::ki64Column) {
        avg = static_cast<Ti64Column*>(col.get())->GetData()[0];
    } else if (col->GetType() == EColumn::kDoubleColumn) {
        avg = static_cast<TDoubleColumn*>(col.get())->GetData()[0];
    } else {
        return MakeError<EError::BadArgsErr>("not an int column");
    }

    avg /= static_cast<ld>(static_cast<Ti64Column*>(cnt.ThrowRowGroup().GetShared().get())->GetData()[0]);

    return std::allocate_shared<TDoubleColumn>(ArenaAlloc(), std::vector<ld>{avg});
}

std::shared_ptr<IOa> TAvgAgr::Clone() {
    auto r = std::allocate_shared<TAvgAgr>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TAvgAgr::GetName() const {
    return "AVG(" + arg->GetName() + ")";
}

} // namespace JfEngine