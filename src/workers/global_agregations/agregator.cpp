#include "agregator.h"

#include "columns/operators/operators.h"

namespace JfEngine {

TAgregator::TAgregator(std::shared_ptr<ITableInput> jf_in, TGlobalAgregationQuery query) :
    jf_in_(std::move(jf_in)),
    eng_(query),
    cols_cnt_(query.cols.size())
{
    for (auto& q : query.cols) {
        blocker_ |= (q->IsBlocker());
    }
}

TAgregator::TAgregator(std::shared_ptr<ITableInput> jf_in) : jf_in_(std::move(jf_in)), is_all_(true) {
}

std::vector<TRowScheme>& TAgregator::GetScheme() {
    if (is_all_) {
        return jf_in_->GetScheme();
    } else {
        return scheme_;
    }
}

Expected<void> TAgregator::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
    auto err = jf_in_->SetupColumnsScheme();
    if (err.HasError()) {
        return err.GetError();
    }
    auto names = eng_.GetNames();
    for (ui64 i = 0; i < cols_cnt_; i++) {
        scheme_.emplace_back(names[i], EColumn::kUnitialized);
    }
    return nullptr;
}

Expected<std::vector<TColumnPtr>> TAgregator::LoadRowGroup() {
    if (!is_all_) {
        bool is_eof = false;
        std::vector<TColumnPtr> ans;
        if (blocker_) {
            bool run = true;
            for (; run; jf_in_->MoveCursor(1)) {
                auto err = eng_.ConsumeRowGroup(jf_in_.get());
                if (err.HasError()) {
                    if (err.GetError() == EError::EofErr) {
                        run = false;
                    } else {
                        return err.GetError();
                    }
                }
            }
            auto [t, _] = eng_.ThrowRowGroup();
            is_eof = true;
            ans = *t;
        } else {
            auto err = eng_.ConsumeRowGroup(jf_in_.get());
            if (err.HasError()) {
                if (err.GetError() != EError::EofErr) {
                    std::cout << "Ffjjfjf" << " " << err.GetError() << std::endl;
                    return err.GetError();
                } else {
                    is_eof = true;
                }
            }
            ans = std::move(*eng_.ThrowRowGroup().GetShared());
        }

        for (ui64 i = 0; i < ans.size(); i++) {
            scheme_[i].type_ = ans[i]->GetType();
        }

        assert(ans.size() == GetScheme().size());
        
        return {std::move(ans), (is_eof ? EError::EofErr : EError::NoError)};
    } else {
        assert(jf_in_->GetScheme().size() == GetScheme().size());
        return jf_in_->ReadRowGroup();
    }
}

void TAgregator::MoveCursor(i64 delta) {
    current_rg_.reset();
    jf_in_->MoveCursor(delta);
}

} // namespace JfEngine