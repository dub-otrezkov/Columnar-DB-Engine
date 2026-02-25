#include "operators.h"

#include "columns/operators/operators.h"

namespace JFEngine {

void IAgregation::AddArg(std::shared_ptr<IAgregation>) {
}

Expected<IColumn> TColumnAgr::ReadRowGroup(ITableInput* inp) {
    return inp->ReadColumn(name);
}

Expected<IColumn> TSumAgr::ReadRowGroup(ITableInput* inp) {
    bool run = 1;
    std::shared_ptr<IColumn> ans = nullptr;
    inp->Reset();

    while (run) {
        auto [col, err] = arg->ReadRowGroup(inp);
        if (err != EError::NoError) {
            if (Is<EError::EofErr>(err)) {
                run = 0;
            } else {
                return err;
            }
        }
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

        inp->MoveCursor(1);
    }
    Expected<IColumn> ret(std::move(ans), MakeError<EError::EofErr>());
    return ret;
}

void TSumAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

Expected<IColumn> TColumnAgr::ReadRowGroup(ITableInput* inp) {
    return inp->ReadColumn(name);
}

TColumnAgr::TColumnAgr(std::string column_name) {
    name = std::move(column_name);
}

void TCountAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

Expected<IColumn> TCountAgr::ReadRowGroup(ITableInput* inp) {
    bool run = 1;
    inp->Reset();

    i64 len = 0;

    while (run) {
        auto [col, err] = arg->ReadRowGroup(inp);
        if (err) {
            if (Is<EofErr>(err)) {
                run = 0;
            } else {
                return err;
            }
        }

        len += col->GetSize();

        inp->MoveCursor(1);
    }
    Expected<IColumn> ret(std::make_shared<Ti64Column>(std::vector<i64>{len}), MakeError<EofErr>());
    return ret;
}

void TAvgAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

Expected<IColumn> TAvgAgr::ReadRowGroup(ITableInput* inp) {
    bool run = 1;
    inp->Reset();

    i64 len = 0;

    TSumAgr sum;
    sum.AddArg(arg);
    TCountAgr cnt;
    cnt.AddArg(arg);

    auto sum_col = sum.ReadRowGroup(inp);

    ld avg = 0;
    if (!Is<EofErr>(sum_col.GetError())) {
        return sum_col.GetError();
    }
    if (sum_col.GetShared()->GetType() == EColumn::ki64Column) {
        avg = static_cast<Ti64Column*>(sum_col.GetShared().get())->GetData()[0];
    } else if (sum_col.GetShared()->GetType() == EColumn::kDoubleColumn) {
        avg = static_cast<TDoubleColumn*>(sum_col.GetShared().get())->GetData()[0];
    } else {
        return MakeError<BadArgsErr>("not an int column");
    }

    auto cnt_col = cnt.ReadRowGroup(inp);

    if (!Is<EofErr>(cnt_col.GetError())) {
        return cnt_col.GetError();
    }

    avg /= static_cast<ld>(static_cast<Ti64Column*>(cnt_col.GetShared().get())->GetData()[0]);

    Expected<IColumn> ret(std::make_shared<TDoubleColumn>(std::vector<ld>{avg}), MakeError<EofErr>());
    return ret;
}

void TSumAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

} // namespace JFEngine