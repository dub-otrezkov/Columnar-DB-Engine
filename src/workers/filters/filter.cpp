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
    auto [col_sp, err] = jf_in_->ReadRowGroup();
    bool is_eof = Is<EError::EofErr>(err);
    if (err && !is_eof) {
        return err;
    }
    auto& col = col_sp;
    if (col.empty()) {
        return {col, err};
    }

    std::vector<bool> keep(col[0]->GetSize(), 1);
    for (const auto& [name, op, target, opt_args] : query_.fils) {
        if (op == EFilterType::kIn || op == EFilterType::kNIn) {
            std::vector<bool> al;
            if (!opt_args) {
                return EError::BadCmdErr;
            }

            for (const auto& item : *opt_args) {
                auto [orr, err] = Do<OFilterCheck>(
                    col[name_to_i_[name]],
                    (op == EFilterType::kIn ? EFilterType::kEq : EFilterType::kNeq),
                    item
                );

                if (err) {
                    return err;
                }

                if (al.empty()) {
                    al = orr;
                } else {
                    for (ui64 i = 0; i < orr.size(); i++) {
                        if (op == EFilterType::kIn) {
                            al[i] = (al[i] | orr.at(i));
                        } else {
                            al[i] = (al[i] & orr.at(i));
                        }
                    }
                }
            }

            for (ui64 i = 0; i < keep.size(); i++) {
                keep[i] = (keep[i] & al[i]);
            }
        } else {
            auto [bl, err] = Do<OFilterCheck>(col[name_to_i_[name]], op, target);

            if (err) {
                return err;
            }

            for (ui64 i = 0; i < keep.size(); i++) {
                keep[i] = (keep[i] & bl[i]);
            }
        }
    }

    std::vector<TColumnPtr> ans(col.size());

    for (ui64 i = 0; i < ans.size(); i++) {
        auto res = Do<OFilter>(col[i], keep);
        if (res.HasError()) {
            return res.GetError();
        }
        if (!ans[i]) {
        }
        ans[i] = res.GetRes();
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