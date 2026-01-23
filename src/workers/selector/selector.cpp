#include "selector.h"

#include <unordered_map>

namespace JFEngine {

TSelector::TSelector(std::istream& jf_in, const std::vector<std::string>& rows) : 
    jf_in_(std::make_unique<TJFTableInput>(jf_in))
{
    scheme_.resize(rows.size());
    for (ui64 i = 0; i < rows.size(); i++) {
        scheme_[i].name_ = rows[i];
    }
}

Expected<void> TSelector::SetupColumnsScheme() {
    auto err = jf_in_->SetupColumnsScheme();

    if (err.HasError()) {
        std::cout << err.GetError().get() << std::endl;
        return err.GetError();
    }

    std::unordered_map<std::string, ui64> inds;
    for (ui64 i = 0; i < scheme_.size(); i++) {
        inds[scheme_[i].name_] = i;
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
        auto [col, err] = jf_in_->ReadColumn(name);
        if (err) {
            return err;
        }
        if (col) {
            res.push_back(*col);
        }
    }
    jf_in_->MoveCursor(1);

    return std::move(res);
}

void TSelector::RestartDataRead() {
    jf_in_->RestartDataRead();
}

} // namespace JFEngine