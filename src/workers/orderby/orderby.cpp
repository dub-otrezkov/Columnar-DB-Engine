#include "workers/orderby/orderby.h"

#include "columns/operators/vector_like.h"
#include "columns/types/types.h"

#include <algorithm>

namespace JfEngine {

TOrderBy::TOrderBy(TTableInputPtr jf_in, TOrderByQuery query) :
    jf_in_(std::move(jf_in)),
    order_q_(std::move(query))
{
    if (order_q_.limit != kUnlimited) {
        order_q_.limit += order_q_.offset;
    }
}

Expected<void> TOrderBy::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
    jf_in_->SetupColumnsScheme();
    scheme_ = jf_in_->GetScheme();
    name_to_i_.clear();
    for (const auto& [name, tp] : scheme_) {
        name_to_i_[name] = name_to_i_.size();
    }
    return EError::NoError;
}

void TOrderBy::SortRowGroup(std::vector<TColumnPtr>& rg, std::vector<TColumnPtr>& other) {
    i64 n1 = rg[0]->GetSize();
    i64 n2 = other[0]->GetSize();
    std::vector<i64> ids(n2);
    for (i64 i = 0; i < ids.size(); i++) {
        ids[i] = i;
    }
    std::vector<i64> is(order_q_.cols.size());
    for (ui64 i = 0; i < is.size(); i++) {
        is[i] = name_to_i_[order_q_.cols.at(i)];
    }

    const i64 dir = order_q_.reverse ? -1 : 1;
    const ui64 nc = order_q_.cols.size();

    std::vector<TIntComparator> cmps_same;
    cmps_same.reserve(nc);
    for (ui64 k = 0; k < nc; k++) {
        cmps_same.push_back(Do<OCmp>(other.at(is[k])));
    }
    auto cmp = [&cmps_same, dir, nc](i64 i, i64 j) -> bool {
        for (ui64 k = 0; k < nc; k++) {
            auto t1 = cmps_same[k](i, j) * dir;
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
    if (order_q_.limit == kUnlimited) {
        std::stable_sort(ids.begin(), ids.end(), cmp);
    } else {
        std::partial_sort(ids.begin(), std::min(ids.end(), ids.begin() + order_q_.limit), ids.end(), cmp);
    }

    std::vector<TIntComparator2> cmps_diff;
    cmps_diff.reserve(nc);
    for (ui64 k = 0; k < nc; k++) {
        cmps_diff.push_back(Do<OCmpDiffCol>(rg.at(is[k]), other.at(is[k])));
    }
    auto cmp2 = [&cmps_diff, dir, nc](i64 i, i64 j) -> bool {
        for (ui64 k = 0; k < nc; k++) {
            auto t1 = cmps_diff[k](i, j) * dir;
            if (t1 == 1) {
                return true;
            }
            if (t1 == 0) {
                continue;
            }
            return false;
        }
        return true;
    };

    std::vector<i64> ans_i;
    if (order_q_.limit == kUnlimited) {
        ans_i.reserve(n1 + n2);
    } else {
        ans_i.reserve(order_q_.limit);
    }
    i64 i = 0;
    i64 j = 0;
    while (i < n1 || j < n2) {
        if (i == n1) {
            ans_i.push_back(-ids[j] - 1);
            j++;
        } else if (j == n2) {
            ans_i.push_back(i);
            i++;
        } else if (cmp2(i, ids[j])) {
            ans_i.push_back(i);
            i++;
        } else {
            ans_i.push_back(-ids[j] - 1);
            j++;
        }

        if (ans_i.size() == order_q_.limit) {
            break;
        }
    }

    for (i64 i = 0; i < rg.size(); i++) {
        auto [res, err] = Do<OApply2>(rg[i], other[i], ans_i);
        if (err) {
            std::cout << "ERROR!" << std::endl;
        } else {
            rg[i] = res;
        }
    }
}

void TOrderBy::MergeRowGroups(
    std::vector<TColumnPtr>& rg1,
    std::vector<TColumnPtr>& rg2
) {
    for (ui64 i = 0; i < rg1.size(); i++) {
        if (!rg1[i]) {
            rg1[i] = MakeEmptyColumn(rg2[i]->GetType()).GetRes();
            scheme_[i].type_ = rg1[i]->GetType();
        }
    }
    SortRowGroup(rg1, rg2);
}


Expected<bool> TOrderBy::ShouldSkipBatch(std::vector<TColumnPtr>& ans) {
    if (ans.empty() || order_q_.limit == kUnlimited || !ans.at(0) || ans.at(0)->GetSize() < order_q_.limit) {
        return false;
    }

    const ui64 nc = order_q_.cols.size();
    std::vector<TColumnPtr> mins(nc);
    bool is_eof = false;

    const i64 dir = order_q_.reverse ? -1 : 1;

    std::vector<i64> is(nc);
    for (ui64 i = 0; i < nc; i++) {
        is.at(i) = name_to_i_.at(order_q_.cols.at(i));
    }

    for (ui64 i = 0; i < nc; i++) {
        auto [mn, err] = jf_in_->ReadMinMax(is.at(i));
        if (err == EError::EofErr) {
            is_eof = true;
        } else if (err != EError::NoError) {
            return false;
        }
        mins.at(i) = std::move(mn);
    }

    std::vector<TIntComparator2> cmps_diff;
    cmps_diff.reserve(nc);
    for (ui64 k = 0; k < nc; k++) {
        cmps_diff.push_back(Do<OCmpDiffCol>(ans.at(is.at(k)), mins.at(k)));
    }
    auto cmp2 = [&cmps_diff, dir, nc](i64 i, i64 j) -> bool {
        for (ui64 k = 0; k < nc; k++) {
            auto t1 = cmps_diff[k](i, j) * dir;
            if (t1 == 1) {
                return true;
            }
            if (t1 == 0) {
                continue;
            }
            return false;
        }
        return true;
    };

    const i64 edge = (dir == 1) ? 0 : 1;
    if (cmp2(ans.at(0)->GetSize() - 1, edge)) {
        return {true, is_eof ? EError::EofErr : EError::NoError};
    }

    return false;
}

Expected<std::vector<TColumnPtr>> TOrderBy::LoadRowGroup() {
    bool run = true;
    
    std::vector<TColumnPtr> ans_;
    ans_.resize(scheme_.size());

    for (; run; jf_in_->MoveCursor()) {
        auto [f, eof] = ShouldSkipBatch(ans_);
        if (f) {
            if (eof == EError::EofErr) {
                run = false;
            }
            continue;
        }

        auto [g, err] = jf_in_->ReadRowGroup();

        if (err) {
            if (err == EError::EofErr) {
                run = false;
            } else {
                return err;
            }
        }
        if (g.empty() || g.at(0)->GetSize() == 0) {
            continue;
        }

        auto& rg = g;

        MergeRowGroups(ans_, rg);
    }

    assert(ans_.size() == GetScheme().size());

    if (order_q_.offset > 0) {
        for (auto& k : ans_) {
            k = Do<OOffset>(k, order_q_.offset).GetRes();
        }
    }

    return {std::move(ans_), EError::EofErr};
}

} // namespace JfEngine
