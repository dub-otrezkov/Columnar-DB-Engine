#include "types.h"

namespace JFEngine {

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

} // namespace JFEngine
