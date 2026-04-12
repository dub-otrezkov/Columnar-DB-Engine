#include "groupby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"

#include <algorithm>

namespace JfEngine {

TGroupBy::TGroupBy(std::shared_ptr<ITableInput> jf_in, TGroupByQuery query, TAoQuery selects) :
    jf_in_(std::move(jf_in)),
    group_q_(std::move(query)),
    agr_q_(selects),
    gc_eng(MakeAoEngine(TAoQuery{group_q_.cols, {}, EAoEngineType::kAgregation}))
{
    scheme_.resize(selects.args.size());
    jf_in_->SetupColumnsScheme();
}

Expected<void> TGroupBy::SetupColumnsScheme() {
    jf_in_->SetupColumnsScheme();
    std::vector<std::string> names(agr_q_.args.size());
    for (ui64 i = 0; i < scheme_.size(); i++) {
        scheme_[i].name_ = agr_q_.args[i]->GetName();
        scheme_[i].type_ = EColumn::kUnitialized;
    }
    for (auto& [i, name] : agr_q_.aliases) {
        scheme_[i].name_ = name;
    }
    return EError::NoError;
}

Expected<std::vector<TColumnPtr>> TGroupBy::LoadRowGroup() {
    bool run = 1;
    std::vector<std::vector<std::string>> changed;

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
                run = 0;
            } else {
                return err2;
            }
        }
        if (!ag || ag->empty() || ag->at(0)->GetSize() == 0) {
            continue;
        }

        auto rg = *g;
        auto sz = rg[0]->GetSize();

        std::vector<std::vector<std::string>> printed(sz, std::vector<std::string>(rg.size()));
        for (ui64 i = 0; i < rg.size(); i++) {
            auto printed_col = Do<OJfPrint>(rg[i]);
            assert(printed_col.size() == sz);
            for (ui64 j = 0; j < sz; j++) {
                printed[j].push_back(std::move(printed_col[j]));
            }
        }

        std::unordered_set<VectorStringHashed, VectorStringHasher> used;

        for (ui64 i = 0; i < sz; i++) {
            VectorStringHashed key(std::move(printed[i]));

            if (!groups_.contains(key)) {
                if (group_q_.limit != kUnlimited && groups_.size() == group_q_.limit) {
                    continue;
                }
                groups_.emplace(key, TGroup{jf_in_->GetScheme(), agr_q_.Clone()});
            }
            groups_.at(key).io.UploadRowGroup(*ag, i);

            used.insert(std::move(key));

        }

        for (const auto& key : used) {
            auto& t = groups_.at(key);
            t.eng->ConsumeRowGroup(&t.io);
            t.io.MoveCursor(); // this clear io (bad naming but i dont care)
        }
    }

    std::vector<TColumnPtr> ans(scheme_.size());
    for (auto&& [_, value] : groups_) {
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