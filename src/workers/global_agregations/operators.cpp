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

TColumnAgr::TColumnAgr(std::string column_name) {
    name = std::move(column_name);
}

void TSumAgr::AddArg(std::shared_ptr<IAgregation> to_sum) {
    arg = to_sum;
}

} // namespace JFEngine