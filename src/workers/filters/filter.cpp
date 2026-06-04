#include "filter.h"

namespace JfEngine {

TFilter::TFilter(TTableInputPtr jf_in, TFilterQuery query) :
    jf_in_(std::move(jf_in)),
    query_(std::move(query))
{
}

Expected<void> TFilter::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
    auto res = jf_in_->SetupColumnsScheme();
    if (res.HasError()) {
        return res;
    }
    scheme_ = jf_in_->GetScheme();
    ui64 i = 0;
    for (const auto& [name, _] : scheme_) {
        name_to_i_[name] = i++;
    }
    return nullptr;
}

Expected<std::vector<TColumnPtr>> TFilter::LoadRowGroup() {
    bool is_eof = false;
    OFilterShouldSkipBatch::EResult pre_check_res = OFilterShouldSkipBatch::EResult::kTakeAll;
    for (const auto& [name, op, target, opt_args] : query_.fils) {
        auto [minmax, err] = jf_in_->ReadMinMax(name);
        if (err == EError::UnimplementedErr) {
            pre_check_res = OFilterShouldSkipBatch::EResult::kNeedCheck;
            continue;
        }
        if (err == EError::EofErr) {
            is_eof = true;
        }
        if (op == EFilterType::kIn || op == EFilterType::kNIn) {
            if (!opt_args) {
                pre_check_res = OFilterShouldSkipBatch::EResult::kNeedCheck;
                continue;
            }
            bool all_absent = true;
            for (const auto& item : *opt_args) {
                auto t = Do<OFilterShouldSkipBatch>(minmax, EFilterType::kEq, item);
                if (t != OFilterShouldSkipBatch::EResult::kSkipAll) {
                    all_absent = false;
                    break;
                }
            }
            OFilterShouldSkipBatch::EResult verdict = OFilterShouldSkipBatch::EResult::kNeedCheck;
            if (all_absent) {
                if (op == EFilterType::kIn) {
                    verdict = OFilterShouldSkipBatch::EResult::kSkipAll;
                } else {
                    verdict = OFilterShouldSkipBatch::EResult::kTakeAll;
                }
            }
            if (verdict == OFilterShouldSkipBatch::EResult::kSkipAll) {
                pre_check_res = verdict;
                break;
            } else if (verdict == OFilterShouldSkipBatch::EResult::kNeedCheck) {
                pre_check_res = verdict;
            }
        } else {
            auto t = Do<OFilterShouldSkipBatch>(minmax, op, target);

            if (t == OFilterShouldSkipBatch::EResult::kSkipAll) {
                pre_check_res = t;
                JF_LOG(nullptr, "skipping" << " " << " " << target << std::endl);
                break;
            } else if (t == OFilterShouldSkipBatch::EResult::kNeedCheck) {
                pre_check_res = t;
            }
        }
    }

    if (pre_check_res == OFilterShouldSkipBatch::EResult::kSkipAll) {
        std::vector<TColumnPtr> empty(scheme_.size());
        for (ui64 i = 0; i < scheme_.size(); i++) {
            auto c = MakeEmptyColumn(scheme_[i].type_);
            if (c.HasError()) {
                return c.GetError();
            }
            empty[i] = c.GetRes();
        }
        return {std::move(empty), is_eof ? MakeError<EError::EofErr>() : EError::NoError};
    }
    if (pre_check_res == OFilterShouldSkipBatch::EResult::kTakeAll) {
        return jf_in_->ReadRowGroup();
    }

    auto [col_sp, err] = jf_in_->ReadRowGroup();
    is_eof = Is<EError::EofErr>(err);
    if (err && !is_eof) {
        return err;
    }
    auto& col = col_sp;
    if (col.empty()) {
        return {col, err};
    }

    boost::dynamic_bitset<> keep(col[0]->GetSize());
    keep.set();
    for (const auto& [name, op, target, opt_args] : query_.fils) {
        if (op == EFilterType::kIn || op == EFilterType::kNIn) {
            if (!opt_args) {
                return EError::BadCmdErr;
            }
            boost::dynamic_bitset<> al(keep.size());
            for (const auto& item : *opt_args) {
                boost::dynamic_bitset<> item_mask(keep.size());
                item_mask.set();
                auto e = Do<OAndCheck>(
                    col[name_to_i_[name]],
                    EFilterType::kEq,
                    item,
                    item_mask
                );
                if (e.HasError()) {
                    return e.GetError();
                }
                al |= item_mask;
            }
            if (op == EFilterType::kIn) {
                keep &= al;
            } else {
                keep -= al;
            }
        } else {
            auto e = Do<OAndCheck>(col[name_to_i_[name]], op, target, keep);
            if (e.HasError()) {
                return e.GetError();
            }
        }
    }

    std::vector<TColumnPtr> ans(col.size());

    for (ui64 i = 0; i < ans.size(); i++) {
        auto res = Do<OFilter>(col[i], keep);
        if (res.HasError()) {
            return res.GetError();
        }
        ans[i] = std::move(res.GetRes());
    }

    assert(ans.size() == GetScheme().size());

    return {std::move(ans), is_eof ? MakeError<EError::EofErr>() : EError::NoError};
}

void TFilter::MoveCursor() {
    current_rg_.reset();
    current_rg_err_ = EError::NoError;
    jf_in_->MoveCursor();
}

} // namespace JfEngine