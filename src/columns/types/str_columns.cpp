#include "types.h"

namespace JfEngine {

TStringColumn::TStringColumn(std::vector<std::string> data) {
    // cols_ = std::move(data);
    cols_.clear();
    cols_.reserve(data.size());
    for (auto& el : data) {
        cols_.emplace_back(std::string_view(el));
    }
}

TStringColumn::TStringColumn(std::vector<JString> data) {
    cols_ = std::move(data);
}

TStringColumn::TStringColumn(std::vector<std::string_view> data) {
    // cols_ = std::move(data);
    cols_.clear();
    cols_.reserve(data.size());
    for (const auto& el : data) {
        cols_.emplace_back(el);
    }
}

EColumn TStringColumn::GetType() const {
    return kStringColumn;
}

Expected<void> TStringColumn::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (ui64 i = 0; i < data.size(); i++) {
        cols_.emplace_back(std::string_view(data[i]));
    }
    return nullptr;
}

Expected<void> TStringColumn::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    for (ui64 i = 0; ; i++) {
        auto [p, err] = data.AtRaw(i, column_i);
        if (err) {
            break;
        }
        auto [ptr, len] = p;
        cols_.emplace_back(len, ptr);
    }
    return nullptr;
}

} // namespace JfEngine
