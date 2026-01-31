#pragma once

#include "utils/errors/errors.h"

namespace JFEngine {

class BadCmdErr : public IError {
public:
    std::string Print() const override {
        return "bad command";
    }
};

} // namespace JFEngine
