#include "operators.h"

#include "columns/operators/operators.h"
#include "columns/operators/min_max.h"
#include "columns/operators/distinct.h"

namespace JFEngine {

void IAgregation::AddArg(std::shared_ptr<IAgregation>) {
}

bool TColumnAgr::IsBlocker() {
    return false;
}

TColumnAgr::TColumnAgr(std::string column_name) {
    name = std::move(column_name);
}

Expected<void> TColumnAgr::ConsumeRowGroup(ITableInput* inp) {
    auto [ans_, err] = inp->ReadColumn(name);
    ans = ans_;
    // std::cout << "(" << name << ")" << " " << this << " " << ans << std::endl;
    return err;
}

Expected<IColumn> TColumnAgr::ThrowRowGroup() {
    // std::cout << ">> " << name << " " << ans->GetSize() << " " << this << std::endl;
    return ans;
}

bool TSumAgr::IsBlocker() {
    return true;
}

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

void TSumAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

void TCountAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

void TMinAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

void TMaxAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

void TDistinctAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

void TLengthAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
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
    if (!arg->IsBlocker()) {
        auto [t, _] = arg->ThrowRowGroup();

        ans += t->GetSize();
    }

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<void> TLengthAgr::ConsumeRowGroup(ITableInput* inp) {
    bool is_eof = false;
    {
        auto err = arg->ConsumeRowGroup(inp);
        if (err.HasError()) {
            if (err.GetError() == EError::EofErr) {
                is_eof = true;
            } else {
                return err.GetError();
            }
        }
    }
    if (!arg->IsBlocker()) {
        auto [t, _] = arg->ThrowRowGroup();

        auto [ans_, err] = Do<OLength>(t);
        if (err) {
            return err;
        }
        ans = std::move(ans_);
    }

    return (is_eof ? EError::EofErr : EError::NoError);
}

bool TCountAgr::IsBlocker() {
    return true;
}

bool TLengthAgr::IsBlocker() {
    return arg->IsBlocker();
}

Expected<IColumn> TCountAgr::ThrowRowGroup() {
    if (arg->IsBlocker()) {
        auto [col, err] = arg->ThrowRowGroup();

        return std::make_shared<Ti64Column>(std::vector<i64>{static_cast<i64>(col->GetSize())});
    } else {
        return std::make_shared<Ti64Column>(std::vector<i64>{ans});
    }
}

void TAvgAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

bool TAvgAgr::IsBlocker() {
    return true;
}

bool TDistinctAgr::IsBlocker() {
    return true;
}

Expected<void> TAvgAgr::ConsumeRowGroup(ITableInput* inp) {
    bool run = 1;
    inp->Reset();

    i64 len = 0;

    if (!inited) {
        sum.AddArg(arg);
        cnt.AddArg(arg);
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

    return std::make_shared<TDoubleColumn>(std::vector<ld>{avg});
}

bool TMinAgr::IsBlocker() {
    return true;
}

bool TMaxAgr::IsBlocker() {
    return true;
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

Expected<void> TDistinctAgr::ConsumeRowGroup(ITableInput* inp) {
    auto err = arg->ConsumeRowGroup(inp);
    bool is_eof = false;
    if (err.GetError() != EError::NoError) {
        if (EError::EofErr == err.GetError()) {
            is_eof = true;
        } else {
            std::cout << "eof " << err.GetError() << std::endl;
            return err;
        }
    }

    auto [col, _] = arg->ThrowRowGroup();

    if (!ans) {
        ans = MakeEmptyColumn(col->GetType()).GetShared();
    }

    auto [ans_, err2] = Do<ODistinct>(ans, col);
    if (err2) {
        std::cout << ":::: " << err2 << std::endl;
        return err2;
    }
    ans = ans_;
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TMinAgr::ThrowRowGroup() {
    return ans;
}

Expected<IColumn> TMaxAgr::ThrowRowGroup() {
    return ans;
}

Expected<IColumn> TDistinctAgr::ThrowRowGroup() {
    return ans;
}

Expected<IColumn> TLengthAgr::ThrowRowGroup() {
    if (arg->IsBlocker()) {
        auto [col, err] = arg->ThrowRowGroup();

        return Do<OLength>(col);
    } else {
        return ans;
    }
}

std::shared_ptr<IAgregation> TColumnAgr::Clone() {
    return std::make_shared<TColumnAgr>(name);
}

std::shared_ptr<IAgregation> TSumAgr::Clone() {
    auto r = std::make_shared<TSumAgr>();
    r->arg = arg->Clone();
    return r;
}

std::shared_ptr<IAgregation> TCountAgr::Clone() {
    auto r = std::make_shared<TCountAgr>();
    r->arg = arg->Clone();
    return r;
}

std::shared_ptr<IAgregation> TAvgAgr::Clone() {
    auto r = std::make_shared<TAvgAgr>();
    r->arg = arg->Clone();
    return r;
}

std::shared_ptr<IAgregation> TMinAgr::Clone() {
    auto r = std::make_shared<TMinAgr>();
    r->arg = arg->Clone();
    return r;
}

std::shared_ptr<IAgregation> TMaxAgr::Clone() {
    auto r = std::make_shared<TMaxAgr>();
    r->arg = arg->Clone();
    return r;
}

std::shared_ptr<IAgregation> TDistinctAgr::Clone() {
    auto r = std::make_shared<TDistinctAgr>();
    r->arg = arg->Clone();
    return r;
}

std::shared_ptr<IAgregation> TLengthAgr::Clone() {
    auto r = std::make_shared<TLengthAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TColumnAgr::GetName() {
    return name;
}

std::string TSumAgr::GetName() {
    return "SUM(" + arg->GetName() + ")";
}

std::string TMinAgr::GetName() {
    return "MIN(" + arg->GetName() + ")";
}

std::string TMaxAgr::GetName() {
    return "MAX(" + arg->GetName() + ")";
}

std::string TCountAgr::GetName() {
    return "COUNT(" + arg->GetName() + ")";
}

std::string TAvgAgr::GetName() {
    return "AVG(" + arg->GetName() + ")";
}

std::string TDistinctAgr::GetName() {
    return "DISTINCT(" + arg->GetName() + ")";
}

std::string TLengthAgr::GetName() {
    return "LENGTH(" + arg->GetName() + ")";
}

} // namespace JFEngine