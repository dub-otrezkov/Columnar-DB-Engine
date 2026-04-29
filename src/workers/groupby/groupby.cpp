#include "groupby.h"

#include "columns/operators/order.h"
#include "columns/operators/vector_like.h"
#include "columns/types/types.h"
#include "utils/perf_stats/perf_stats.h"

#include <algorithm>

namespace JfEngine {

TGroupBy::TGroupBy(TTableInputPtr jf_in, TGroupByQuery query, TAoQuery selects) :
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

    const char* gc_eng_name = "AgregEngine";
    const char* per_group_eng_name = agr_q_.tp == EAoEngineType::kAgregation
        ? "AgregEngine" : "OperEngine";
    
    
    boost::unordered_flat_map<std::string, i64> in_name_to_i;
    // std::cout << "start -------------" << std::endl;
    for (i64 i = 0; i < jf_in_->GetScheme().size(); i++) {
        // std::cout << jf_in_->GetScheme()[i].name_ << std::endl;
        in_name_to_i[jf_in_->GetScheme()[i].name_] = i;
    }
    // std::cout << "split -------------" << std::endl;
    auto n = gc_eng->GetNames();
    std::vector<i64> is(n.size());
    for (ui64 i = 0; i < n.size(); i++) {
        // std::cout << n.at(i) << std::endl;
        is.at(i) = in_name_to_i.at(n.at(i));
    }
    // std::cout << "end ---------------" << std::endl;

    auto cmp = [&](std::vector<TColumnPtr>& input, i64 i, i64 j) -> bool {

        for (i64 k = 0; k < n.size(); k++) {
            auto t1 = Do<OCmp>(input.at(is[k]), i, j);
            if (t1 == 1) {
                return true;
            }
            if (t1 == 0) {
                continue;
            }
            return false;
        }
        return false;
    };

    auto equ = [&](std::vector<TColumnPtr>& input, i64 i, i64 j) -> bool {

        for (i64 k = 0; k < n.size(); k++) {
            auto t1 = Do<OCmp>(input.at(is[k]), i, j);
            if (t1 != 0) {
                return false;
            }
        }
        return true;
    };

    auto compute_order = [&](std::vector<TColumnPtr>& input) -> std::vector<i64> {
        std::vector<i64> ids(input[0]->GetSize());
        for (i64 i = 0; i < (i64)ids.size(); i++) ids[i] = i;
        std::sort(ids.begin(), ids.end(), [&](i64 i, i64 j) { return cmp(input, i, j); });
        return ids;
    };

    auto apply_order = [&](std::vector<TColumnPtr>& cols, const std::vector<i64>& ids) {
        for (auto& col : cols)
            col = Do<OApplyOrder>(col, ids).GetRes();
    };

    for (; run; jf_in_->MoveCursor()) {
        std::vector<TColumnPtr> g;
        {
            TAoEngineTimer t(gc_eng_name);
            (void)gc_eng->ConsumeRowGroup(jf_in_.get());
            g = gc_eng->ThrowRowGroup();
        }

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

        {
            auto ids = compute_order(ag);
            apply_order(ag, ids);
            apply_order(g, ids);
        }

        if (!inp_) {
            inp_ = TNarrowTableInput();
            inp_->Update(jf_in_->GetScheme());
        }

        auto& rg = g;
        const ui64 sz = rg[0]->GetSize();

        ui64 lst = 0;

        for (ui64 i = 0; i < sz; i++) {
            if (i + 1 == sz || !equ(ag, i, i + 1)) {
                inp_->MoveCursor();
                inp_->UploadRowGroup(ag, lst, i);

                StringVector key_data;
                key_data.reserve(rg.size());
                for (ui64 c = 0; c < rg.size(); c++) {
                    Do<OJfPrintRow>(rg[c], i, key_data);
                }
                VectorStringHashed key(std::move(key_data));

                auto it = groups_.find(key);
                if (it == groups_.end()) {
                    it = groups_.emplace(key, TGroup{agr_q_.Clone()}).first;
                }
                TAoEngineTimer t(per_group_eng_name);
                it->second.eng->ConsumeRowGroup(&inp_.value());

                lst = i + 1;
            }
        }

        // for (ui64 i = 0; i < sz; i++) {
        //     RowView view{&rg, i, row_hashes_[i]};
        //     auto it = keys.find(view);

        //     if (it == keys.end()) {
        //         if (group_q_.limit != kUnlimited && groups_.size() >= group_q_.limit) {
        //             continue;
        //         }

        //         StringVector key_data;
        //         key_data.reserve(rg.size());
        //         for (ui64 c = 0; c < rg.size(); c++) {
        //             Do<OJfPrintRow>(rg[c], i, key_data);
        //         }
        //         VectorStringHashed key(std::move(key_data), row_hashes_[i]);
        //         it = keys.emplace(std::move(key), std::vector<ui64>(0)).first;
        //     }
        //     it->second.push_back(i);
        //     // inp_->MoveCursor();
        //     // inp_->UploadRowGroup(*ag, i);
        //     // it->second.eng->ConsumeRowGroup(&inp_.value());
        // }

        // for (auto& [key, is] : keys) {
        //     inp_->MoveCursor();
        //     inp_->UploadRowGroup(ag, is);

        //     auto it = groups_.find(key);
        //     if (it == groups_.end()) {
        //         it = groups_.emplace(key, TGroup{agr_q_.Clone()}).first;
        //     }
        //     TAoEngineTimer t(per_group_eng_name);
        //     it->second.eng->ConsumeRowGroup(&inp_.value());
        // }
    }

    std::vector<TColumnPtr> ans(scheme_.size());
    for (auto& [_, value] : groups_) {
        if (!ans[0]) {
            {
                TAoEngineTimer t(per_group_eng_name);
                ans = value.eng->ThrowRowGroup();
            }
            for (ui64 i = 0; i < ans.size(); i++) {
                Do<OResize>(ans[i], 1);
                scheme_[i].type_ = ans[i]->GetType();
            }
        } else {
            std::vector<TColumnPtr> gars;
            {
                TAoEngineTimer t(per_group_eng_name);
                gars = value.eng->ThrowRowGroup();
            }
            for (ui64 i = 0; i < gars.size(); i++) {
                Do<OPushBackFrom>(gars[i], ans[i], 0);
            }
        }
    }

    assert(ans.size() == GetScheme().size());
    return {std::move(ans), EError::EofErr};
}

} // namespace JfEngine