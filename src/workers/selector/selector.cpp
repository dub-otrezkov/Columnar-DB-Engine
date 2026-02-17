#include "selector.h"

#include <vector>

namespace JFEngine {

TSelector::TSelector(std::shared_ptr<TJFTableInput> jf_in, TSelectQuery query) : jf_in_(jf_in) {
    aliases_ = std::move(query.aliases);
    scheme_.resize(query.rows.size());
    for (ui64 i = 0; i < scheme_.size(); i++) {
        scheme_[i].name_ = query.rows[i];
        if (aliases_.count(scheme_[i].name_)) {
            scheme_[i].name_ = aliases_[scheme_[i].name_];
        }
    }

    for (const auto& [name, _] : scheme_) {
        unaliases_[name] = name;
    }

    for (const auto& [name, alias] : aliases_) {
        unaliases_[alias] = name;
    }
}

Expected<void> TSelector::SetupColumnsScheme() {
    auto err = jf_in_->SetupColumnsScheme();

    if (err.HasError()) {
        return err.GetError();
    }

    std::unordered_map<std::string, ui64> inds;
    for (ui64 i = 0; i < scheme_.size(); i++) {
        inds[unaliases_[scheme_[i].name_]] = i;
    }

    auto prev_scheme = jf_in_->GetScheme();

    for (const auto& [name, tp] : prev_scheme) {
        if (inds.count(name) > 0) {
            scheme_[inds[name]].type_ = tp;
            inds.erase(name);
        }
    }

    if (!inds.empty()) {
        return MakeError<NoSuchColumnsErr>(std::move(inds));
    }

    return nullptr;
}

std::vector<TRowScheme>& TSelector::GetScheme() {
    return scheme_;
}


Expected<std::vector<TColumnPtr>> TSelector::ReadRowGroup() {
    bool is_eof = false;
    std::vector<TColumnPtr> res;

    for (const auto& [name, _] : scheme_) {
        auto [col, err] = jf_in_->ReadColumn(unaliases_.at(name));
        if (err) {
            return err;
        }
        if (col) {
            res.push_back(col);
        }
    }
    jf_in_->MoveCursor(1);

    return std::move(res);
}

} // namespace JFEngine