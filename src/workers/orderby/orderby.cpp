#include "workers/orderby/orderby.h"

#include "columns/operators/vector_like.h"
#include "columns/operators/order.h"
#include "columns/types/types.h"

#include <algorithm>

namespace JfEngine {

TOrderBy::TOrderBy(std::shared_ptr<ITableInput> jf_in, TOrderByQuery query) :
    jf_in_(std::move(jf_in)),
    order_q_(std::move(query))
{
    jf_in_->SetupColumnsScheme();
}

Expected<void> TOrderBy::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
    jf_in_->SetupColumnsScheme();
    scheme_ = jf_in_->GetScheme();
    name_to_i_.clear();
    for (auto [name, tp] : scheme_) {
        name_to_i_[name] = name_to_i_.size();
    }
    return EError::NoError;
}

void TOrderBy::SortRowGroup(std::vector<TColumnPtr>& rg, ui64 column) {
    auto order = Do<OSort>(rg[column], order_q_.reverse);
    for (auto& ptr : rg) {
        auto [col, err] = Do<OApplyOrder>(ptr, order);
        if (!err) {
            ptr = col;
        }
    }
}

void TOrderBy::SortRowGroup(std::vector<TColumnPtr>& rg) {
    for (i64 i = order_q_.cols.size(); i > 0; i--) {
        SortRowGroup(rg, name_to_i_[order_q_.cols[i - 1]]);
    }
}

void TOrderBy::MergeRowGroups(
    std::vector<TColumnPtr>& rg1,
    std::vector<TColumnPtr>& rg2
) {
    for (ui64 i = 0; i < rg1.size(); i++) {
        // std::cout << "++:" << " " << rg2[i] << " " << rg1[i] << std::endl;
        if (!rg1[i]) {
            rg1[i] = MakeEmptyColumn(rg2[i]->GetType()).GetShared();
            scheme_[i].type_ = rg1[i]->GetType();
        }
        Do<OPushBackVector>(rg2[i], rg1[i]);
    }
    SortRowGroup(rg1);
    if (order_q_.limit != kUnlimited && rg1[0]->GetSize() > order_q_.limit) {
        for (ui64 i = 0; i < rg1.size(); i++) {
            Do<OResize>(rg1[i], order_q_.limit);
        }
    }
}

Expected<std::vector<TColumnPtr>> TOrderBy::LoadRowGroup() {
    bool run = 1;
    
    std::vector<TColumnPtr> ans_;
    ans_.resize(scheme_.size());
    // for (auto [name, tp] : scheme_) {
    //     // std::cout << 
    //     ans_.push_back(MakeEmptyColumn(tp).GetShared());
    //     std::cout << ":: : " << ans_.back() << " " << tp << std::endl;
    // }

    for (; run; jf_in_->MoveCursor()) {
        std::vector<std::vector<std::string>> keys;
        auto [g, err] = jf_in_->ReadRowGroup();
        // jf_in_->MoveCursor(1);

        if (err) {
            if (err == EError::EofErr) {
                run = 0;
            } else {
                return err;
            }
        }
        if (!g || g->empty() || g->at(0)->GetSize() == 0) {
            continue;
        }

        auto rg = *g;

        // std::cout << "merge q: " << " " << ans_ << " " << rg << std::endl; 
        MergeRowGroups(ans_, rg);
    }

    assert(ans_.size() == GetScheme().size());

    return {std::move(ans_), EError::EofErr};
}

} // namespace JfEngine