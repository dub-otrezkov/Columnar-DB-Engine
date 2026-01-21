#pragma once

#include "utils/errors/errors.h"

namespace JFEngine {

class NotAnIntErr : public IError {
public:
    std::string Print() const override {
        return "NotAnIntErr";
    }
};

class IntOverflowErr : public IError {
public:
    std::string Print() const override {
        return "Overflow";
    }
};

} // namespace JFEngine
