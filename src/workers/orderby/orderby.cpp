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
    auto cmp = [&](i64 i, i64 j) -> bool {
        for (i64 k = 0; k < order_q_.cols.size(); k++) {
            auto t1 = Do<OCmp>(other.at(is[k]), i, j) * (order_q_.reverse ? -1 : 1);
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
    if (order_q_.limit == kUnlimited) {
        std::stable_sort(ids.begin(), ids.end(), cmp);
    } else {
        std::partial_sort(ids.begin(), std::min(ids.end(), ids.begin() + order_q_.limit), ids.end(), cmp);
    }
    
    auto cmp2 = [&](i64 i, i64 j) -> bool {
        for (i64 k = 0; k < order_q_.cols.size(); k++) {
            auto t1 = Do<OCmpDiffCol>(rg.at(is[k]), other.at(is[k]), i, j) * (order_q_.reverse ? -1 : 1);
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
    // if (order_q_.limit != kUnlimited && !rg1.empty() && rg1[0]->GetSize() > order_q_.limit) {
    //     for (ui64 j = 0; j < rg1.size(); j++) {
    //         Do<OResize>(rg1[j], order_q_.limit);
    //     }
    // }
}

Expected<std::vector<TColumnPtr>> TOrderBy::LoadRowGroup() {
    bool run = 1;
    
    std::vector<TColumnPtr> ans_;
    ans_.resize(scheme_.size());

    for (; run; jf_in_->MoveCursor()) {
        auto [g, err] = jf_in_->ReadRowGroup();

        if (err) {
            if (err == EError::EofErr) {
                run = 0;
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