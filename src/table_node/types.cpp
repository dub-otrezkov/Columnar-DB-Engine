#include "types.h"

IError* Ti64Node::Set(const std::string& data) {
    try {
        value_ = std::stoll(data);
        return nullptr;
    } catch (std::invalid_argument) {
        return new NotAnIntErr;
    } catch (std::out_of_range) {
        return new IntOverflowErr;
    }
}

std::string Ti64Node::Get() {
    return std::to_string(value_);
}

IError* TStringNode::Set(const std::string& data) {
    value_ = data;
    return nullptr;
}

std::string TStringNode::Get() {
    return value_;
}