#include "narrow_io.h"

#include "columns/operators/vector_like.h"

namespace JFEngine {
    
TNarrowTableInput::TNarrowTableInput(std::vector<TRowScheme>& scheme) {
    scheme_ = scheme;
    for (const auto& [name, _] : scheme_) {
        name_to_i_[name] = name_to_i_.size();
    }
    buf_ = std::make_shared<std::vector<TColumnPtr>>(scheme_.size());

    for (ui64 i = 0; i < scheme_.size(); i++) {
        buf_->at(i) = MakeEmptyColumn(scheme_[i].type_).GetShared();
    }
}

Expected<void> TNarrowTableInput::SetupColumnsScheme() {
    throw "dont do that";
}

void TNarrowTableInput::MoveCursor(i64 delta) {
    current_rg_.reset();
    for (ui64 i = 0; i < scheme_.size(); i++) {
        buf_->at(i) = MakeEmptyColumn(scheme_[i].type_).GetShared();
    }
}

Expected<std::vector<TColumnPtr>> TNarrowTableInput::LoadRowGroup() {
    return buf_;
}

void TNarrowTableInput::UploadRowGroup(std::vector<TColumnPtr>& row_group, ui64 row_i) {
    ui64 i = 0;
    for (auto& col : row_group) {
        Do<OPushBackFrom>(col, buf_->at(i++), row_i);
    }
}

} // namespace JFEngine