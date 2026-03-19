#include "groupby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"

#include <algorithm>

namespace JFEngine {

TGroupBy::TGroupBy(std::shared_ptr<ITableInput> jf_in, TGroupByQuery query, TGlobalAgregationQuery selects) :
    jf_in_(std::move(jf_in)),
    scheme_(selects.cols.size()),
    group_q_(std::move(query)),
    agr_q_(selects)
{
    jf_in_->SetupColumnsScheme();
}

Expected<void> TGroupBy::SetupColumnsScheme() {
    std::vector<std::string> names(agr_q_.cols.size());
    for (ui64 i = 0; i < scheme_.size(); i++) {
        scheme_[i].name_ = agr_q_.cols[i]->GetName();
        scheme_[i].type_ = EColumn::kUnitialized;
    }
    for (auto& [i, name] : agr_q_.aliases) {
        scheme_[i].name_ = name;
    }
    return EError::NoError;
}

std::vector<TRowScheme>& TGroupBy::GetScheme() {
    return scheme_;
}

Expected<std::vector<TColumnPtr>> TGroupBy::ReadRowGroup() {
    bool run = 1;
    std::vector<std::vector<std::string>> changed;

    if (name_to_i_.empty()) {
        for (const auto& k : jf_in_->GetScheme()) {
            name_to_i_[k.name_] = name_to_i_.size();
        }
    }

    while (run) {
        std::vector<std::vector<std::string>> keys;
        auto [g, err] = jf_in_->ReadRowGroup();

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
        auto sz = rg[0]->GetSize();

        for (ui64 i = 0; i < sz; i++) {
            std::vector<std::string> key;
            key.reserve(group_q_.cols.size());
            for (const auto& k : group_q_.cols) {
                key.push_back(Do<OJFPrintIth>(rg[name_to_i_[k]], i));
            }

            if (!groups_.contains(key)) {
                if (group_q_.limit != kUnlimited && groups_.size() == group_q_.limit) {
                    continue;
                }
                groups_.emplace(key, TGroup{jf_in_->GetScheme(), agr_q_.Clone()});
            }
            groups_.at(key).io.UploadRowGroup(rg, i);

            keys.push_back(std::move(key));
        }

        std::sort(keys.begin(), keys.end());
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

        // for (auto k : keys) {
        //     std::cout << "|";
        //     for (auto el : k) {
        //         std::cout << el << " ";
        //     }
        //     std::cout << std::endl;
        // }

        for (const auto& key : keys) {
            auto& t = groups_.at(key);
            t.eng.ConsumeRowGroup(&t.io);
            t.io.ReadRowGroup(); // this clear io (bad naming but i dont care)
        }
    }
    std::vector<TColumnPtr> ans(scheme_.size());
    for (auto& [_, value] : groups_) {
        if (!ans[0]) {
            auto [t, _] = value.eng.ThrowRowGroup();
            ans = *t;
            for (ui64 i = 0; i < ans.size(); i++) {
                scheme_[i].type_ = ans[i]->GetType();
            }
        } else {
            auto [tgars, _] = value.eng.ThrowRowGroup();
            auto gars = *tgars;
            for (ui64 i = 0; i < gars.size(); i++) {
                Do<OPushBackFrom>(gars[i], ans[i], 0);
            }
        }
    }

    return {std::move(ans), EError::EofErr};
}

} // namespace JFEngine