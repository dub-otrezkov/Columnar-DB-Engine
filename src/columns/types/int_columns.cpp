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

Ti128Column::Ti128Column(std::vector<i128> data) {
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

EColumn Ti128Column::GetType() const {
    return ki128Column;
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

// GEMINI CODE STARTED

// Перегрузка для знакового i128
Ti128Column::from_chars_result_128 Ti128Column::from_chars(const char* first, const char* last, i128& value, int base) {
    if (first == last) return {first, std::errc::invalid_argument};

    bool negative = false;
    const char* start = first;

    if (*first == '-') {
        negative = true;
        start++;
    }

    i128 u_val;
    auto res = from_chars(start, last, u_val, base);

    if (res.ec != std::errc{}) return res;

    // Проверка диапазона для знакового типа
    if (negative) {
        if (u_val > std::numeric_limits<i128>::max()) {
            return {res.ptr, std::errc::result_out_of_range};
        }
        value = -u_val;
    } else {
        if (u_val > std::numeric_limits<i128>::max()) {
            return {res.ptr, std::errc::result_out_of_range};
        }
        value = u_val;
    }

    return res;
}

// GEMINI CODE END

Expected<void> Ti128Column::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        cols_.push_back(0);
        from_chars(s.data(), s.data() + s.size(), cols_.back());
    }
    return nullptr;
}

Expected<void> Ti128Column::Setup(const TVectorString2d& data, ui64 column_i) {
    cols_.reserve(data.Size());
    std::string cur;
    for (ui64 i = 0; !data.At(i, column_i, &cur).HasError(); i++) {
        cols_.push_back(0);
        from_chars(cur.data(), cur.data() + cur.size(), cols_.back());
        // cols_.push_back(std::stoll(cur));
    }
    return nullptr;
}

} // namespace JfEngine
