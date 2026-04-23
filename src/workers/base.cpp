#include "base.h"

namespace JfEngine {

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

Expected<std::vector<TColumnPtr>> ITableInput::ReadRowGroup() {
    if (!current_rg_) {
        auto result = LoadRowGroup();
        current_rg_err_ = result.GetError();
        if (result.HasValue()) {
            current_rg_ = std::make_shared<std::vector<TColumnPtr>>(std::move(result.GetRes()));
        }
    }
    if (current_rg_) {
        return {std::vector<TColumnPtr>(*current_rg_), current_rg_err_};
    }
    return current_rg_err_;
}

Expected<TColumnPtr> ITableInput::ReadColumn(const std::string& name) {
    if (!current_rg_) {
        auto result = LoadRowGroup();
        current_rg_err_ = result.GetError();
        if (result.HasValue()) {
            current_rg_ = std::make_shared<std::vector<TColumnPtr>>(std::move(result.GetRes()));
        }
    }
    if (!current_rg_) {
        return current_rg_err_;
    }
    if (name == "*") {
        return Expected<TColumnPtr>{(*current_rg_)[0], current_rg_err_};
    }

    if (name_to_i_.empty()) {
        for (const auto& [n, _] : scheme_) {
            name_to_i_[n] = name_to_i_.size();
        }
    }

    if (!name_to_i_.contains(name)) {
        return MakeError<EError::NoSuchColumnsErr>("no such column " + name);
    }

    return Expected<TColumnPtr>{(*current_rg_)[name_to_i_[name]], current_rg_err_};
}

ui64 ITableInput::GetRowGroupLen() const {
    return row_group_len_;
}

void ITableInput::Reset() {
}

void ITableInput::MoveCursor() {
    current_rg_.reset();
    current_rg_err_ = EError::NoError;
}

std::vector<TRowScheme>& ITableInput::GetScheme() {
    return scheme_;
}

} // namespace JfEngine