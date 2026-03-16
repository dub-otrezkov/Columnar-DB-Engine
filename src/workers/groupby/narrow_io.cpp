#include "narrow_io.h"

#include "columns/operators/vector_like.h"

namespace JFEngine {
    
TNarrowTableInput::TNarrowTableInput(std::vector<TRowScheme>& scheme) {
    buf_.resize(scheme.size());

    for (const auto& [name, _] : scheme) {
        name_to_i_[name] = name_to_i_.size();
    }

    for (ui64 i = 0; i < scheme.size(); i++) {
        buf_[i] = MakeEmptyColumn(scheme[i].type_).GetShared();
    }
}

Expected<void> TNarrowTableInput::SetupColumnsScheme() {
    throw "dont do that";
}

std::vector<TRowScheme>& TNarrowTableInput::GetScheme() {
    throw "dont do that";
}

Expected<std::vector<TColumnPtr>> TNarrowTableInput::ReadRowGroup() {
    for (auto& el : buf_) {
        el = MakeEmptyColumn(el->GetType()).GetShared();
    }
    return EError::NoError;
}

Expected<IColumn> TNarrowTableInput::ReadColumn(const std::string& name) {
    auto i = name_to_i_[name];
    if (buf_.size() <= i) {
        return EError::EofErr;
    }
    return buf_[i];
}

void TNarrowTableInput::UploadRowGroup(std::vector<TColumnPtr>& row_group, ui64 row_i) {
    ui64 i = 0;
    for (auto& col : row_group) {
        Do<OPushBackFrom>(col, buf_[i++], row_i);
    }
}

} // namespace JFEngine