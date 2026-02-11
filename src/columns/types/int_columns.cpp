#include "types.h"

namespace JFEngine {

Ti64Column::Ti64Column(std::vector<i64> data) {
    cols_ = std::move(data);
}

EColumn Ti8Column::GetType() {
    return Ei8Column;
}

EColumn Ti16Column::GetType() {
    return Ei16Column;
}

EColumn Ti32Column::GetType() {
    return Ei32Column;
}

EColumn Ti64Column::GetType() {
    return Ei64Column;
}

Expected<void> Ti8Column::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            cols_.push_back(static_cast<i8>(std::stoi(s)));
        } catch (...) {
            return MakeError<NotAnIntErr>();
        }
    }
    return nullptr;
}

Expected<void> Ti16Column::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            cols_.push_back(static_cast<i16>(std::stoi(s)));
        } catch (...) {
            return MakeError<NotAnIntErr>();
        }
    }
    return nullptr;
}

Expected<void> Ti32Column::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            cols_.push_back(static_cast<i32>(std::stoi(s)));
        } catch (...) {
            return MakeError<NotAnIntErr>();
        }
    }
    return nullptr;
}

Expected<void> Ti64Column::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            cols_.push_back(std::stoll(s));
        } catch (...) {
            return MakeError<NotAnIntErr>();
        }
    }
    return nullptr;
}

} // namespace JFEngine
