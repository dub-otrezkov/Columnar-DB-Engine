#include "base.h"

namespace JfEngine {

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

Expected<std::vector<TColumnPtr>> ITableInput::ReadRowGroup() {
    if (!current_rg_) {
        current_rg_ = LoadRowGroup();
    }
    return *current_rg_;
}

Expected<IColumn> ITableInput::ReadColumn(const std::string& name) {
    if (!current_rg_) {
        current_rg_ = LoadRowGroup();
    }
    if (name == "*") {
        return Expected<IColumn>{current_rg_->GetRes()[0], current_rg_->GetError()};
    }

    if (name_to_i_.empty()) {
        for (auto [name, _] : scheme_) {
            name_to_i_[name] = name_to_i_.size();
        }
    }

    if (!name_to_i_.contains(name)) {
        return EError::NoSuchColumnsErr;
    }

    // std::cout << "!! " << (current_rg_->GetShared()) << std::endl;

    return Expected<IColumn>{current_rg_->GetRes()[name_to_i_[name]], current_rg_->GetError()};
}

ui64 ITableInput::GetRowGroupLen() const {
    return row_group_len_;
}

void ITableInput::Reset() {
}

void ITableInput::MoveCursor(i64) {
    current_rg_.reset();
}

ui64 ITableInput::GetGroupsCount() const {
    return 0;
}

std::vector<TRowScheme>& ITableInput::GetScheme() {
    return scheme_;
}

} // namespace JfEngine