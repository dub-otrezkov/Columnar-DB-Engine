#include "narrow_io.h"

#include "columns/operators/vector_like.h"

#include <cassert>

namespace JfEngine {
    
void TNarrowTableInput::Update(std::vector<TRowScheme>& scheme) {
    scheme_ = scheme;
    name_to_i_.clear();
    for (const auto& [name, _] : scheme_) {
        name_to_i_[name] = name_to_i_.size();
    }
    buf_ = std::make_shared<std::vector<TColumnPtr>>(scheme_.size());

    for (ui64 i = 0; i < scheme_.size(); i++) {
        buf_->at(i) = MakeEmptyColumn(scheme_[i].type_).GetShared();
    }
}

Expected<void> TNarrowTableInput::SetupColumnsScheme() {
    return EError::NoError;
}

void TNarrowTableInput::MoveCursor() {
    current_rg_.reset();
    for (ui64 i = 0; i < scheme_.size(); i++) {
        buf_->at(i) = MakeEmptyColumn(scheme_[i].type_).GetShared();
    }
}

Expected<std::vector<TColumnPtr>> TNarrowTableInput::LoadRowGroup() {
    assert(buf_->size() == GetScheme().size());
    return buf_;
}

void TNarrowTableInput::UploadRowGroup(std::vector<TColumnPtr>& row_group, std::vector<ui64>& row_i) {
    ui64 i = 0;
    for (auto& col : row_group) {
        Do<OPushBackFromBatch>(col, buf_->at(i++), row_i);
    }
}

} // namespace JfEngine