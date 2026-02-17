#include "types.h"

namespace JFEngine {

EColumn TDateColumn::GetType() {
    return kDateColumn;
}

EColumn TTimestampColumn::GetType() {
    return kTimestampColumn;
}

Expected<void> TDateColumn::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            // YYYY-MM-DD
            if (s.size() != 10) {
                throw std::runtime_error("");
            }
            cols_.emplace_back(
                std::stoi(s.substr(0, 4)),
                std::stoi(s.substr(5, 2)),
                std::stoi(s.substr(8, 2))
            );
        } catch (...) {
            return MakeError<NotAnDateErr>();
        }
    }
    return nullptr;
}

Expected<void> TTimestampColumn::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            // YYYY-MM-DD HH:MM:SS
            if (s.size() != 19) {
                throw std::runtime_error("");
            }
            cols_.emplace_back(
                TDate{
                    static_cast<i16>(std::stoi(s.substr(0, 4))),
                    static_cast<i8>(std::stoi(s.substr(5, 2))),
                    static_cast<i8>(std::stoi(s.substr(8, 2)))
                },
                std::stoi(s.substr(11, 2)),
                std::stoi(s.substr(14, 2)),
                std::stoi(s.substr(17, 2))
            );
        } catch (...) {
            return MakeError<NotAnTimestampErr>();
        }
    }
    return nullptr;
}

} // namespace JFEngine