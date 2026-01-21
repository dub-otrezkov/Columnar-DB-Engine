#pragma once

#include <utils/errors/errors.h>

namespace JFEngine {

class IncorrectFileErr : public IError {
public:
    IncorrectFileErr(std::string message = "bad input") : message_(std::move(message)) {
    }

    std::string Print() const override {
        return message_;
    }

private:
    std::string message_;
};

class UnimplementedErr : public IError {
public:

    std::string Print() const override {
        return "unimplemented";
    }
};

} // namespace JFEngine
