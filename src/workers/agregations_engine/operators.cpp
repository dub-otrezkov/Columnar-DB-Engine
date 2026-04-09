#include "operators.h"

#include "columns/operators/operators.h"
#include "columns/operators/min_max.h"
#include "columns/operators/distinct.h"

#include <cassert>

namespace JfEngine {

void IAgregation::AddArg(std::shared_ptr<IAgregation>) {
}

TColumnAgr::TColumnAgr(std::string column_name) {
    name = std::move(column_name);
}

bool TColumnAgr::IsBlocker() {
    return false;
}

Expected<void> TColumnAgr::ConsumeRowGroup(ITableInput* inp) {
    auto [ans_, err] = inp->ReadColumn(name);
    ans = ans_;
    return err;
}

Expected<IColumn> TColumnAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IAgregation> TColumnAgr::Clone() {
    return std::make_shared<TColumnAgr>(name);
}

std::string TColumnAgr::GetName() {
    return name;
}

void TSumAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
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

std::shared_ptr<IAgregation> TSumAgr::Clone() {
    auto r = std::make_shared<TSumAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TSumAgr::GetName() {
    return "SUM(" + arg->GetName() + ")";
}

void TCountAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

bool TCountAgr::IsBlocker() {
    return true;
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

Expected<IColumn> TCountAgr::ThrowRowGroup() {
    if (arg->IsBlocker()) {
        auto [col, err] = arg->ThrowRowGroup();

        return std::make_shared<Ti64Column>(std::vector<i64>{static_cast<i64>(col->GetSize())});
    } else {
        return std::make_shared<Ti64Column>(std::vector<i64>{ans});
    }
}

std::shared_ptr<IAgregation> TCountAgr::Clone() {
    auto r = std::make_shared<TCountAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TCountAgr::GetName() {
    return "COUNT(" + arg->GetName() + ")";
}

void TMinAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

bool TMinAgr::IsBlocker() {
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

Expected<IColumn> TMinAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IAgregation> TMinAgr::Clone() {
    auto r = std::make_shared<TMinAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TMinAgr::GetName() {
    return "MIN(" + arg->GetName() + ")";
}

void TMaxAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

bool TMaxAgr::IsBlocker() {
    return true;
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

std::shared_ptr<IAgregation> TMaxAgr::Clone() {
    auto r = std::make_shared<TMaxAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TMaxAgr::GetName() {
    return "MAX(" + arg->GetName() + ")";
}

void TAvgAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

bool TAvgAgr::IsBlocker() {
    return true;
}

Expected<void> TAvgAgr::ConsumeRowGroup(ITableInput* inp) {
    bool run = 1;

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

std::shared_ptr<IAgregation> TAvgAgr::Clone() {
    auto r = std::make_shared<TAvgAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TAvgAgr::GetName() {
    return "AVG(" + arg->GetName() + ")";
}

void TDistinctAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

bool TDistinctAgr::IsBlocker() {
    return true;
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
        return err2;
    }
    ans = ans_;
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TDistinctAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IAgregation> TDistinctAgr::Clone() {
    auto r = std::make_shared<TDistinctAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TDistinctAgr::GetName() {
    return "DISTINCT(" + arg->GetName() + ")";
}

void TLengthAgr::AddArg(std::shared_ptr<IAgregation> to_agr) {
    arg = to_agr;
}

bool TLengthAgr::IsBlocker() {
    return arg->IsBlocker();
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

Expected<IColumn> TLengthAgr::ThrowRowGroup() {
    if (arg->IsBlocker()) {
        auto [col, err] = arg->ThrowRowGroup();

        return Do<OLength>(col);
    } else {
        return ans;
    }
}

std::shared_ptr<IAgregation> TLengthAgr::Clone() {
    auto r = std::make_shared<TLengthAgr>();
    r->arg = arg->Clone();
    return r;
}

std::string TLengthAgr::GetName() {
    return "LENGTH(" + arg->GetName() + ")";
}

Expected<void> TPlusAgr::ConsumeRowGroup(ITableInput* inp) {
    bool is_eof = false;

    assert(args.size() == 2); // <- she needs him?

    // std::cout << "Fklfkflf" << std::endl;

    auto err = args[0]->ConsumeRowGroup(inp).GetError();

    if (err == EError::EofErr) {
        is_eof = true;
    }

    auto [ans_, _] = args[0]->ThrowRowGroup();


    for (ui64 i = 1; i < args.size(); i++) {
        auto err1 = args[i]->ConsumeRowGroup(inp).GetError();
        // std::cout << "::: " << err1 << std::endl;
        if (err1 == EError::NoError || err1 == EError::EofErr) {
            // std::cout << "lflflf" << std::endl;
            auto [c, err2] = Do<OVerticalSum>(ans_, args[i]->ThrowRowGroup().GetShared());
            ans_ = std::move(c);
            if (err1 == EError::EofErr) {
                is_eof = true;
            }
        } else if (err1 == EError::NoSuchColumnsErr) {
            std::cout << "dkdkkdkd" << std::endl;
            auto [c, err2] = Do<OAddConst>(ans_, args[i]->GetName());
            ans_ = std::move(c);
        }
    }

    ans = ans_;

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TPlusAgr::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IAgregation> TPlusAgr::Clone() {
    auto r = std::make_shared<TPlusAgr>();
    for (auto& arg : args) {
        r->AddArg(arg->Clone());
    }
    return std::move(r);
}

bool TPlusAgr::IsBlocker() {
    return false;
}

std::string TPlusAgr::GetName() {
    std::string ans = "+(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return std::move(ans);
}

void TPlusAgr::AddArg(std::shared_ptr<IAgregation> arg) {
    args.push_back(std::move(arg));
}

Expected<IColumn> TMinusAgr::ThrowRowGroup() {
    return ans;
}

Expected<void> TMinusAgr::ConsumeRowGroup(ITableInput* inp) {
    bool is_eof = false;

    assert(args.size() == 2); // <- she needs him?

    auto err = args[0]->ConsumeRowGroup(inp).GetError();

    if (err == EError::EofErr) {
        is_eof = true;
    }

    auto [ans_, _] = args[0]->ThrowRowGroup();


    for (ui64 i = 1; i < args.size(); i++) {
        auto err1 = args[i]->ConsumeRowGroup(inp).GetError();
        if (err1 == EError::NoError || err1 == EError::EofErr) {
            auto [c, err2] = Do<OVerticalSub>(ans_, args[i]->ThrowRowGroup().GetShared());
            ans_ = std::move(c);
            if (err1 == EError::EofErr) {
                is_eof = true;
            }
        } else if (err1 == EError::NoSuchColumnsErr) {
            auto [c, err2] = Do<OSubConst>(ans_, args[i]->GetName());
            ans_ = std::move(c);
        }
    }

    ans = ans_;

    return (is_eof ? EError::EofErr : EError::NoError);
}

std::shared_ptr<IAgregation> TMinusAgr::Clone() {
    auto r = std::make_shared<TMinusAgr>();
    for (auto& arg : args) {
        r->AddArg(arg->Clone());
    }
    return std::move(r);
}

bool TMinusAgr::IsBlocker() {
    return false;
}

void TMinusAgr::AddArg(std::shared_ptr<IAgregation> arg) {
    args.push_back(std::move(arg));
}

std::string TMinusAgr::GetName() {
    std::string ans = "-(";
    for (auto& arg : args) {
        ans += arg->GetName() + " ";
    }
    ans.back() = ')';
    return std::move(ans);
}

} // namespace JfEngine