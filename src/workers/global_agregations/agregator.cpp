#include "agregator.h"

#include "columns/operators/operators.h"

namespace JFEngine {

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
    for (ui64 i = 0; i < cols_cnt_; i++) {
        scheme_.emplace_back("column " + std::to_string(i), EColumn::kUnitialized);
    }
    return nullptr;
}

Expected<std::vector<TColumnPtr>> TAgregator::ReadRowGroup() {
    if (!is_all_) {
        bool is_eof = false;
        std::vector<TColumnPtr> ans;
        if (blocker_) {
            jf_in_->Reset();
            for (ui64 i = 0; i < jf_in_->GetGroupsCount(); i++) {
                eng_.ConsumeRowGroup(jf_in_.get());

                jf_in_->MoveCursor(1);
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
            // }
            jf_in_->MoveCursor(1);
        }

        for (ui64 i = 0; i < ans.size(); i++) {
            scheme_[i].type_ = ans[i]->GetType();
        }
        
        return {std::move(ans), (is_eof ? EError::EofErr : EError::NoError)};
    } else {
        // std::cout << "fjkfjkfk" << std::endl;
        return jf_in_->ReadRowGroup();
    }
    // std::vector<TColumnPtr> ans;
    // auto is_eof = 0;
    // for (ui64 i = 0; i < query_.cols.size(); i++) {
    //     auto col = query_.cols[i]->ReadRowGroup(jf_in_.get());
    //     if (col.HasError()) {
    //         if (!Is<EError::EofErr>(col.GetError())) {
    //             return col.GetError();
    //         } else {
    //             is_eof = 1;
    //         }
    //     }

    //     if (!col.GetShared()) {
    //         continue;
    //     }

    //     ans.push_back(col.GetShared());
    //     scheme_[i].type_ = col.GetShared()->GetType();
    // }

    // jf_in_->MoveCursor(1);

    // Expected<std::vector<TColumnPtr>> ret(std::move(ans), is_eof ? MakeError<EError::EofErr>() : EError::NoError);
    // return ret;
}

} // namespace JFEngine