#pragma once

#include "utils/errors/errors.h"

namespace JFEngine {

class IONotFoundErr : public IError {
public:
    IONotFoundErr(const std::string& alias) : alias_(alias) {
    }

    std::string Print() const override {
        return "no io with alias: " + alias_;
    }
private:
    std::string alias_;
};

} // namespace JFEngine