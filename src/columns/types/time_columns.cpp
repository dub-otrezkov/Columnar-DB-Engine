#include "types.h"

namespace JfEngine {

TDateColumn::TDateColumn(std::vector<TDate> data) {
    cols_ = std::move(data);
}

TTimestampColumn::TTimestampColumn(std::vector<TTimestamp> data) {
    cols_ = std::move(data);
}

EColumn TDateColumn::GetType() {
    return kDateColumn;
}

EColumn TTimestampColumn::GetType() {
    return kTimestampColumn;
}

Expected<void> TDateColumn::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        try {
            // YYYY-MM-DD
            if (s.size() != 10) {
                throw std::runtime_error("");
            }
            cols_.push_back(DateFromStr(s));
        } catch (...) {
            return MakeError<EError::NotAnDateErr>();
        }
    }
    return nullptr;
}

Expected<void> TDateColumn::Setup(const TVectorString2d& data, ui64 column_i) {
    // cols_.reserve(data.size());
    std::string s;
    for (ui64 i = 0; !data.At(i, column_i, &s).HasError(); i++) {
        try {
            // YYYY-MM-DD
            if (s.size() != 10) {
                throw std::runtime_error("");
            }
            cols_.push_back(DateFromStr(s));
        } catch (...) {
            return MakeError<EError::NotAnDateErr>();
        }
    }
    return nullptr;
}

Expected<void> TTimestampColumn::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    try {
        for (const auto& s : data) {
            // YYYY-MM-DD HH:MM:SS
            if (s.size() != 19) {
                throw std::runtime_error("");
            }
            cols_.emplace_back(TimestampFromStr(s));
        }
    } catch (...) {
        return MakeError<EError::NotAnTimestampErr>();
    }
    return nullptr;
}

Expected<void> TTimestampColumn::Setup(const TVectorString2d& data, ui64 column_i) {
    // cols_.reserve(data.size());
    std::string s;
    try {
        for (ui64 i = 0; !data.At(i, column_i, &s).HasError(); i++) {
            // YYYY-MM-DD HH:MM:SS
            if (s.size() != 19) {
                throw std::runtime_error("");
            }
            cols_.emplace_back(TimestampFromStr(s));
        }
    } catch (...) {
        return MakeError<EError::NotAnTimestampErr>();
    }
    return nullptr;
}

} // namespace JfEngine