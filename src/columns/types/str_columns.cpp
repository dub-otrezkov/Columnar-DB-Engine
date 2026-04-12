#include "types.h"

namespace JfEngine {

TStringColumn::TStringColumn(std::vector<std::string> data) {
    // cols_ = std::move(data);
    cols_.clear();
    for (const auto& el : data) {
        cols_.push_back(el);
    }
}

TStringColumn::TStringColumn(StringVector data) {
    cols_ = std::move(data);
}

TStringColumn::TStringColumn(std::vector<std::string_view> data) {
    // cols_ = std::move(data);
    cols_.clear();
    for (const auto& el : data) {
        cols_.push_back(std::string{el});
    }
}

EColumn TStringColumn::GetType() const {
    return kStringColumn;
}

Expected<void> TStringColumn::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (ui64 i = 0; i < data.size(); i++) {
        cols_.push_back(std::move(data[i]));
    }
    return nullptr;
}

Expected<void> TStringColumn::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        cols_.push_back(cur);
    }
    return nullptr;
}

} // namespace JfEngine
