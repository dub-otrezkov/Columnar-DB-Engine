#include "types.h"

#include <charconv>

namespace JfEngine {

Ti8Column::Ti8Column(std::vector<i8> data) {
    cols_ = std::move(data);
}

Ti16Column::Ti16Column(std::vector<i16> data) {
    cols_ = std::move(data);
}

Ti32Column::Ti32Column(std::vector<i32> data) {
    cols_ = std::move(data);
}

Ti64Column::Ti64Column(std::vector<i64> data) {
    cols_ = std::move(data);
}

EColumn Ti8Column::GetType() const {
    return ki8Column;
}

EColumn Ti16Column::GetType() const {
    return ki16Column;
}

EColumn Ti32Column::GetType() const {
    return ki32Column;
}

EColumn Ti64Column::GetType() const {
    return ki64Column;
}

Expected<void> Ti8Column::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        cols_.push_back(0);
        std::from_chars(s.data(), s.data() + s.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti8Column::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        cols_.push_back(0);
        std::from_chars(cur.data(), cur.data() + cur.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti16Column::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        cols_.push_back(0);
        std::from_chars(s.data(), s.data() + s.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti16Column::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        cols_.push_back(0);
        std::from_chars(cur.data(), cur.data() + cur.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti32Column::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        cols_.push_back(0);
        std::from_chars(s.data(), s.data() + s.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti32Column::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        cols_.push_back(0);
        std::from_chars(cur.data(), cur.data() + cur.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti64Column::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        cols_.push_back(0);
        std::from_chars(s.data(), s.data() + s.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti64Column::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        cols_.push_back(0);
        std::from_chars(cur.data(), cur.data() + cur.size(), cols_.back());
        // cols_.push_back(std::stoll(cur));
    }
    return nullptr;
}

} // namespace JfEngine
