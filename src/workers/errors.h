#pragma once

#include "utils/errors/errors.h"

#include <unordered_map>

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

class NoSuchColumnsErr : public IError {
public:
    NoSuchColumnsErr(std::unordered_map<std::string, ui64> cols) : cols_(std::move(cols)) {
    }
    NoSuchColumnsErr(const std::string& col) {
        cols_[col] = 0;
    }

    std::string Print() const override {
        std::string ans = "no such columns: ";
        for (const auto& [name, _] : cols_) {
            ans += name + ", ";
        }
        ans.pop_back();
        ans.pop_back();

        return ans;
    }

private:
    std::unordered_map<std::string, ui64> cols_;
};

} // namespace JFEngine
