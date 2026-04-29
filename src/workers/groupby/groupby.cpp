#include "groupby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"
#include "utils/perf_stats/perf_stats.h"

#include <algorithm>
#include <numeric>

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

    const char* gc_eng_name = "AgregEngine";
    const char* per_group_eng_name = agr_q_.tp == EAoEngineType::kAgregation
        ? "AgregEngine" : "OperEngine";

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

        // Sort indices by group-by columns (g), no in-place column copies
        const ui64 sz = g[0]->GetSize();
        std::vector<ui64> ids(sz);
        std::iota(ids.begin(), ids.end(), 0);
        std::sort(ids.begin(), ids.end(), [&](ui64 a, ui64 b) {
            for (auto& col : g) {
                auto t = Do<OCmp>(col, static_cast<i64>(a), static_cast<i64>(b));
                if (t == 1) return true;
                if (t == 0) continue;
                return false;
            }
            return false;
        });

        if (!inp_) {
            inp_ = TNarrowTableInput();
            inp_->Update(jf_in_->GetScheme());
        }

        ui64 lst = 0;

        for (ui64 i = 0; i < sz; i++) {
            bool last_in_group = (i + 1 == sz);
            if (!last_in_group) {
                bool same = true;
                for (auto& col : g) {
                    auto t = Do<OCmp>(col, static_cast<i64>(ids[i]), static_cast<i64>(ids[i + 1]));
                    if (t != 0) { same = false; break; }
                }
                if (same) continue;
            }

            std::vector<ui64> group_rows(ids.begin() + lst, ids.begin() + i + 1);
            inp_->MoveCursor();
            inp_->UploadRowGroup(ag, group_rows);

            StringVector key_data;
            key_data.reserve(g.size());
            for (ui64 c = 0; c < g.size(); c++) {
                Do<OJfPrintRow>(g[c], static_cast<i64>(ids[i]), key_data);
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