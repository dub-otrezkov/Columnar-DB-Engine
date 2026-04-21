#include "groupby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"

#include <algorithm>

namespace JfEngine {

TGroupBy::TGroupBy(std::shared_ptr<ITableInput> jf_in, TGroupByQuery query, TAoQuery selects) :
    jf_in_(std::move(jf_in)),
    group_q_(std::move(query)),
    agr_q_(std::move(selects)),
    gc_eng(MakeAoEngine(TAoQuery{{}, std::move(group_q_.cols), {}, EAoEngineType::kAgregation}))
{
}

Expected<void> TGroupBy::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return EError::NoError;
    }
    jf_in_->SetupColumnsScheme();
    groups_.clear();
    for (ui64 i = 0; i < agr_q_.args.size(); i++) {
        if (agr_q_.args[i]->is_final) {
            scheme_.emplace_back(agr_q_.args[i]->GetName(), EColumn::kUnitialized);
        }
    }
    for (auto& [i, name] : agr_q_.aliases) {
        scheme_[i].name_ = name;
    }
    return EError::NoError;
}

Expected<std::vector<TColumnPtr>> TGroupBy::LoadRowGroup() {
    bool run = true;

    if (name_to_i_.empty()) {
        for (const auto& k : jf_in_->GetScheme()) {
            name_to_i_[k.name_] = name_to_i_.size();
        }
    }

    for (; run; jf_in_->MoveCursor()) {
        auto err = gc_eng->ConsumeRowGroup(jf_in_.get()).GetError();
        auto [g, _] = gc_eng->ThrowRowGroup();

        auto [ag, err2] = jf_in_->ReadRowGroup();

        if (err2) {
            if (err2 == EError::EofErr) {
                run = false;
            } else {
                return err2;
            }
        }

        if (!ag || ag->empty() || ag->at(0)->GetSize() == 0) {
            continue;
        }

        if (!inp_) {
            inp_ = TNarrowTableInput();
            inp_->Update(jf_in_->GetScheme());
        }

        auto rg = *g;
        const ui64 sz = rg[0]->GetSize();

        row_hashes_.assign(sz, 0);
        for (ui64 c = 0; c < rg.size(); c++) {
            Do<OHashInto>(rg[c], row_hashes_);
        }
        for (ui64 i = 0; i < sz; i++) {
            row_hashes_[i] = Finalize(row_hashes_[i]);
        }

        keys.clear();

        for (ui64 i = 0; i < sz; i++) {
            RowView view{&rg, i, row_hashes_[i]};
            auto it = keys.find(view);

            if (it == keys.end()) {
                if (group_q_.limit != kUnlimited && groups_.size() >= group_q_.limit) {
                    continue;
                }

                StringVector key_data;
                key_data.reserve(rg.size());
                for (ui64 c = 0; c < rg.size(); c++) {
                    Do<OJfPrintRow>(rg[c], i, key_data);
                }
                VectorStringHashed key(std::move(key_data), row_hashes_[i]);
                it = keys.emplace(std::move(key), std::vector<ui64>(0)).first;
            }
            it->second.push_back(i);
            // inp_->MoveCursor();
            // inp_->UploadRowGroup(*ag, i);
            // it->second.eng->ConsumeRowGroup(&inp_.value());
        }

        for (auto& [key, is] : keys) {
            inp_->MoveCursor();
            inp_->UploadRowGroup(*ag, is);

            auto it = groups_.find(key);
            if (it == groups_.end()) {
                it = groups_.emplace(key, TGroup{agr_q_.Clone()}).first;
            }
            it->second.eng->ConsumeRowGroup(&inp_.value());
        }
    }

    std::vector<TColumnPtr> ans(scheme_.size());
    for (auto& [_, value] : groups_) {
        if (!ans[0]) {
            auto [t, _] = value.eng->ThrowRowGroup();
            ans = *t;
            for (ui64 i = 0; i < ans.size(); i++) {
                Do<OResize>(ans[i], 1);
                scheme_[i].type_ = ans[i]->GetType();
            }
        } else {
            auto [tgars, _] = value.eng->ThrowRowGroup();
            auto gars = *tgars;
            for (ui64 i = 0; i < gars.size(); i++) {
                Do<OPushBackFrom>(gars[i], ans[i], 0);
            }
        }
    }

    assert(ans.size() == GetScheme().size());
    return {std::move(ans), EError::EofErr};
}

} // namespace JfEngine