#pragma once

#include "utils/errors/errors.h"
#include "utils/cint/int.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace JFEngine {

template<typename T>
class TStorage {
public:
    std::vector<T>& GetData() {
        return col_;
    }
protected:
    std::vector<T> col_;
};

class IColumn {
public:
    virtual ~IColumn() = default;
    virtual Expected<void> Set(const std::string& data) = 0;
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

class Ti64Column : public IColumn, public TStorage<i64> {
public:
    Expected<void> Set(const std::string& data) override;
    std::string Get() override;
private:
    i64 value_;
};

class TStringColumn : public IColumn, public TStorage<std::string> {
public:
    Expected<void> Set(const std::string& data) override;
    std::string Get() override;

private:
    std::string value_;
};


class DynamicCastErr : public IError {
public:
    std::string Print() const override {
        return "NotAnIntErr";
    }
};

class OPushBack {
public:
    
};
