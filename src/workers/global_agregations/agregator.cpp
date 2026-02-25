#include "agregator.h"

#include "columns/operators/operators.h"

namespace JFEngine {

TAgregator::TAgregator(std::shared_ptr<ITableInput> jf_in, TGlobalAgregationQuery query) :
    jf_in_(std::move(jf_in)),
    query_(std::move(query))
{
}

std::vector<TRowScheme>& TAgregator::GetScheme() {
    return scheme_;
}

Expected<void> TAgregator::SetupColumnsScheme() {
    for (ui64 i = 0; i < query_.cols.size(); i++) {
        scheme_.emplace_back("column " + std::to_string(i), EColumn::kUnitialized);
    }
    return nullptr;
}

Expected<std::vector<TColumnPtr>> TAgregator::ReadRowGroup() {
    std::vector<TColumnPtr> ans;
    auto is_eof = 0;
    for (ui64 i = 0; i < query_.cols.size(); i++) {
        auto col = query_.cols[i]->ReadRowGroup(jf_in_.get());
        if (col.HasError()) {
            if (!Is<EError::EofErr>(col.GetError())) {
                return col.GetError();
            } else {
                is_eof = 1;
            }
        }

        ans.push_back(col.GetShared());
        scheme_[i].type_ = col.GetShared()->GetType();
    }

    jf_in_->MoveCursor(1);

    Expected<std::vector<TColumnPtr>> ret(std::move(ans), is_eof ? MakeError<EError::EofErr>() : EError::NoError);
    return ret;
}

} // namespace JFEngine