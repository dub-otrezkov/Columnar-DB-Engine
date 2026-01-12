#include "utils/errors/errors.h"

#include "utils/cint/int.h"

#include <stdexcept>

#include <string>

class ITableNode {
public:
    virtual ~ITableNode() = default;
    virtual IError* Set(const std::string& data) = 0;
    virtual std::string Get() = 0;
};

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

class Ti64Node : public ITableNode {
public:
    IError* Set(const std::string& data) override;
    std::string Get() override;
private:
    i64 value_;
};

class TStringNode : public ITableNode {
public:
    IError* Set(const std::string& data) override;
    std::string Get() override;
private:
    std::string value_;
};