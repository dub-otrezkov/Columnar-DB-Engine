#include "types.h"

namespace JfEngine {

TDoubleColumn::TDoubleColumn(std::vector<ld> data) {
    cols_ = std::move(data);
}

EColumn TDoubleColumn::GetType() {
    return kDoubleColumn;
}

Expected<void> TDoubleColumn::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        try {
            cols_.push_back(std::stold(s));
        } catch (...) {
            return MakeError<EError::NotAnIntErr>();
        }
    }
    return nullptr;
}

Expected<void> TDoubleColumn::Setup(const TVectorString2d& data, ui64 column_i) {
    // cols_.reserve(data.size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        try {
            cols_.push_back(std::stold(cur));
        } catch (...) {
            return MakeError<EError::NotAnIntErr>();
        }
    }
    return nullptr;
}

} // namespace JfEngine
