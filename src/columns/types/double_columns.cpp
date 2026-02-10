#include "types.h"

namespace JFEngine {

TColumn TDoubleColumn::GetType() {
    return EDoubleColumn;
}

Expected<void> TDoubleColumn::Setup(std::vector<std::string> data) {
    for (const auto& s : data) {
        try {
            cols_.push_back(std::stold(s));
        } catch (...) {
            return MakeError<NotAnIntErr>();
        }
    }
    return nullptr;
}

} // namespace JFEngine
