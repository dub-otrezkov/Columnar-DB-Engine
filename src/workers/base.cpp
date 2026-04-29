#include "base.h"

#include "utils/perf_stats/perf_stats.h"

#include <chrono>

namespace JfEngine {

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

void ITableInput::EnsureLoaded() {
    if (current_rg_) return;
    if (!TQueryStats::instance) {
        auto result = LoadRowGroup();
        current_rg_err_ = result.GetError();
        if (result.HasValue()) {
            current_rg_ = std::make_shared<std::vector<TColumnPtr>>(std::move(result.GetRes()));
        }
        return;
    }
    auto t0 = std::chrono::steady_clock::now();
    PushChildFrame();
    auto result = LoadRowGroup();
    auto ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - t0).count());
    uint64_t child_ns = PopChildFrame();
    TQueryStats::instance->Record(GetTypeName(), ns > child_ns ? ns - child_ns : 0);
    AddToParentChildTime(ns);
    current_rg_err_ = result.GetError();
    if (result.HasValue()) {
        current_rg_ = std::make_shared<std::vector<TColumnPtr>>(std::move(result.GetRes()));
    }
}

Expected<std::vector<TColumnPtr>> ITableInput::ReadRowGroup() {
    EnsureLoaded();
    if (current_rg_) {
        return {std::vector<TColumnPtr>(*current_rg_), current_rg_err_};
    }
    return current_rg_err_;
}

Expected<TColumnPtr> ITableInput::ReadColumn(const std::string& name) {
    EnsureLoaded();
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


i64 ITableInput::GetColumnInd(const std::string& name) {
    if (name == "*") {
        return 0;
    }

    if (name_to_i_.empty()) {
        for (const auto& [n, _] : scheme_) {
            name_to_i_[n] = name_to_i_.size();
        }
    }

    if (!name_to_i_.contains(name)) {
        return -1;
    }

    return name_to_i_.at(name);
}

Expected<TColumnPtr> ITableInput::ReadIthColumn(i64 i) {
    EnsureLoaded();
    if (!current_rg_) {
        return current_rg_err_;
    }
    if (i < 0 || i >= current_rg_->size()) {
        return MakeError<EError::NoSuchColumnsErr>();
    }
    return Expected<TColumnPtr>{(*current_rg_)[i], current_rg_err_};
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