#pragma once

#include "utils/errors/errors.h"

class EofErr : public IError {
public:
    std::string Print() const override {
        return "EOF";
    }
};
