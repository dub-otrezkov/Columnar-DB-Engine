#include "types.h"

namespace JFEngine {

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

Expected<void> TTimestampColumn::Setup(std::vector<std::string>&& data) {
    cols_.reserve(data.size());
    for (const auto& s : data) {
        try {
            // YYYY-MM-DD HH:MM:SS
            if (s.size() != 19) {
                throw std::runtime_error("");
            }
            cols_.emplace_back(TimestampFromStr(s));
        } catch (...) {
            return MakeError<EError::NotAnTimestampErr>();
        }
    }
    return nullptr;
}

} // namespace JFEngine