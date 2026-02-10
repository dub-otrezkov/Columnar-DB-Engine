#pragma once

#include "errors.h"
#include "utils/cint/int.h"

#include <unordered_map>

namespace JFEngine {

class NotAnIntErr : public IError {
public:
    std::string Print() const override {
        return "NotAnIntErr";
    }
};

class NotAnDateErr : public IError {
public:
    std::string Print() const override {
        return "NotADateErr";
    }
};


class NotAnTimestampErr : public IError {
public:
    std::string Print() const override {
        return "NotATimestampErr";
    }
};

class IntOverflowErr : public IError {
public:
    std::string Print() const override {
        return "Overflow";
    }
};

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

class BadCmdErr : public IError {
public:
    BadCmdErr(std::string alias = "") : alias_(std::move(alias)) {
    }

    std::string Print() const override {
        return "bad command: " + alias_;
    }
private:
    std::string alias_;
};

class UnsupportedErr : public IError {
public:
    std::string Print() const override {
        return "unsupperted op";
    }
};

class EofErr : public IError {
public:
    std::string Print() const override {
        return "EOF";
    }
};

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
