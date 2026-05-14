#include "groupby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"

#include "utils/logger/logger.h"

#include <algorithm>

namespace JfEngine {

TGroupBy::TGroupBy(TTableInputPtr jf_in, TGroupByQuery query, TAoQuery selects) :
    jf_in_(std::move(jf_in)),
    group_q_(std::move(query)),
    eng_(MakeAoEngine(std::move(selects)))
{
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

        if (!inp_) {
            inp_ = TNarrowTableInput();
            inp_->Update(jf_in_->GetScheme());
        }

        const ui64 sz = ag[0]->GetSize();

        keys.clear();

        std::vector<std::vector<JString>> printed;
        printed.reserve(group_q_.cols.size());
        for (auto& c : group_q_.cols) {
            auto idx = jf_in_->GetColumnInd(c);
            if (idx < 0 || static_cast<ui64>(idx) >= ag.size()) {
                return MakeError<EError::NoSuchColumnsErr>(c);
            }
            printed.push_back(Do<OToJStrings>(ag[idx]));
        }

        std::vector<JString> key;
        for (ui64 i = 0; i < sz; i++) {
            key.resize(group_q_.cols.size());
            for (ui64 j = 0; j < key.size(); j++) {
                key[j] = printed[j][i];
            }
            auto it = keys.find(key);

            if (it == keys.end()) {
                if (group_q_.limit != kUnlimited && groups_.size() >= group_q_.limit) {
                    continue;
                }
                it = keys.emplace(key, std::vector<ui64>(0)).first;
            }
            it->second.push_back(i);
        }

        for (auto& [key, is] : keys) {
            inp_->MoveCursor();
            inp_->UploadRowGroup(ag, is);

            auto it = groups_.find(key);

            if (it == groups_.end()) {
                it = groups_.emplace(key, groups_.size()).first;
            }
            eng_->ConsumeRowGroup(&inp_.value(), it->second);
        }
    }

    std::vector<TColumnPtr> ans = eng_->ThrowRowGroup();
    // for (auto& [_, value] : groups_) {
    //     if (!ans[0]) {
    //         ans = value.eng->ThrowRowGroup();
    //         for (ui64 i = 0; i < ans.size(); i++) {
    //             Do<OResize>(ans[i], 1);
    //             scheme_[i].type_ = ans[i]->GetType();
    //         }
    //     } else {
    //         auto gars = value.eng->ThrowRowGroup();
    //         for (ui64 i = 0; i < gars.size(); i++) {
    //             Do<OPushBackFrom>(gars[i], ans[i], 0);
    //         }
    //     }
    // }

    assert(ans.size() == GetScheme().size());
    return {std::move(ans), EError::EofErr};
}

} // namespace JfEngine