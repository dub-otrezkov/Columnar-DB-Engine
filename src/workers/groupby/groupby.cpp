#include "groupby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"

#include "utils/logger/logger.h"

#include <algorithm>

namespace JfEngine {

TGroupBy::TGroupBy(TTableInputPtr jf_in, TGroupByQuery query, TAoQuery selects) :
    jf_in_(std::move(jf_in)),
    group_q_(std::move(query))
{
    selects.tp = EAoEngineType::kAgregation;
    eng_ = MakeAoEngine(std::move(selects));
}

Expected<void> TGroupBy::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return EError::NoError;
    }
    jf_in_->SetupColumnsScheme();
    groups_.clear();
    for (auto name : eng_->GetNames()) {
        scheme_.emplace_back(name, EColumn::kUnitialized);
    }
    col_idxs_.clear();
    col_idxs_.reserve(group_q_.cols.size());
    for (auto& c : group_q_.cols) {
        col_idxs_.push_back(jf_in_->GetColumnInd(c));
    }
    return EError::NoError;
}

Expected<std::vector<TColumnPtr>> TGroupBy::LoadRowGroup() {
    bool run = true;

    for (; run; jf_in_->MoveCursor()) {
        auto [ag, err2] = jf_in_->ReadRowGroup();

        if (err2) {
            if (err2 == EError::EofErr) {
                run = false;
            } else {
                return err2;
            }
        }

        if (ag.empty() || ag.at(0)->GetSize() == 0) {
            continue;
        }

        const ui64 sz = ag[0]->GetSize();
        std::vector<ui64> idcs(sz);

        TRowView view{&ag, &col_idxs_, 0};
        for (ui64 i = 0; i < sz; i++) {
            view.row = i;
            auto it = groups_.find(view);
            if (it == groups_.end()) {
                if (group_q_.limit != kUnlimited && groups_.size() >= group_q_.limit) {
                    continue;
                }
                TStoredKey k;
                k.vals.reserve(view.ColsCnt());
                for (ui64 c = 0; c < view.ColsCnt(); c++) {
                    auto [p, len] = view.BytesAt(c);
                    k.vals.emplace_back(len, p);
                }
                it = groups_.emplace(std::move(k), groups_.size()).first;
            }
            idcs[i] = it->second;
        }

        eng_->ConsumeRowGroup(jf_in_.get(), &idcs);
    }

    std::vector<TColumnPtr> ans = eng_->ThrowRowGroup();

    assert(ans.size() == GetScheme().size());
    return {std::move(ans), EError::EofErr};
}

} // namespace JfEngine
