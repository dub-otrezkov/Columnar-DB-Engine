#include "operators.h"

#include "columns/operators/operators.h"
#include "columns/operators/min_max.h"

#include <cassert>

namespace JfEngine {

Expected<void> TPlusOp::ConsumeRowGroup(ITableInput* inp) {
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
            auto [c, err2] = Do<OVerticalSum>(ans_, args[i]->ThrowRowGroup().GetShared());
            ans_ = std::move(c);
            if (err1 == EError::EofErr) {
                is_eof = true;
            }
        } else if (err1 == EError::NoSuchColumnsErr) {
            auto [c, err2] = Do<OAddConst>(ans_, args[i]->GetName());
            ans_ = std::move(c);
        }
    }

    ans = ans_;

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TPlusOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TPlusOp::Clone() {
    auto r = std::allocate_shared<TPlusOp>(ArenaAlloc());
    for (auto& arg : args) {
        r->AddArg(arg->Clone());
    }
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

std::shared_ptr<IOa> TMinusOp::Clone() {
    auto r = std::allocate_shared<TMinusOp>(ArenaAlloc());
    for (auto& arg : args) {
        r->AddArg(arg->Clone());
    }
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

    auto [t, _] = arg->ThrowRowGroup();

    auto [ans_, err] = Do<OLength>(t);
    if (err) {
        return err;
    }
    ans = std::move(ans_);

    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TLengthOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TLengthOp::Clone() {
    auto r = std::allocate_shared<TLengthOp>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TLengthOp::GetName() const {
    return "LENGTH(" + arg->GetName() + ")";
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
    return std::allocate_shared<TColumnOp>(ArenaAlloc(), name);
}

std::string TColumnOp::GetName() const {
    return name;
}

Expected<void> TDistinctOp::ConsumeRowGroup(ITableInput* inp) {
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
    ans = Do<ODistinctStreamV>(col, cur_sets).GetShared();
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<IColumn> TDistinctOp::ThrowRowGroup() {
    return ans;
}

std::shared_ptr<IOa> TDistinctOp::Clone() {
    auto r = std::allocate_shared<TDistinctOp>(ArenaAlloc());
    r->arg = arg->Clone();
    return r;
}

std::string TDistinctOp::GetName() const {
    return "DISTINCT(" + arg->GetName() + ")";
}

} // namespace JfEngine
