#include "types.h"

namespace JFEngine {

Expected<void> Ti64Column::Set(const std::string& data) {
    try {
        value_ = std::stoll(data);
        return nullptr;
    } catch (std::invalid_argument) {
        return MakeError<NotAnIntErr>();
    } catch (std::out_of_range) {
        return MakeError<IntOverflowErr>();
    }
}

std::string Ti64Column::Get() {
    return std::to_string(value_);
}

Expected<void> TStringColumn::Set(const std::string& data) {
    value_ = data;
    return nullptr;
}

std::string TStringColumn::Get() {
    return value_;
}

} // namespace JFEngine
