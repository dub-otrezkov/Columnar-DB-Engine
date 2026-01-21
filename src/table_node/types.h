#pragma once

#include "utils/errors/errors.h"
#include "utils/cint/int.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace JFEngine {

enum TColumn {
    EUnitialized,
    Ei64Column,
    EStringColumn
};

class IColumn {
public:
    virtual ~IColumn() = default;

    virtual ui64 GetSize() = 0;

    void SetType(TColumn t) {
        type_ = t;
    }
    TColumn GetType() {
        return type_;
    }

private:
    TColumn type_ = EUnitialized;
};

template <typename T>
class TStorage : public IColumn {
public:
    
    ui64 GetSize() override {
        return cols_.size();
    }

    std::vector<T>& GetData() {
        return cols_;
    }

    virtual Expected<void> Setup(std::vector<std::string> data) = 0;

protected:
    std::vector<T> cols_;
};

class Ti64Column : public TStorage<i64> {
public:
    Ti64Column() {}

    Expected<void> Setup(std::vector<std::string> data) override;
};

class TStringColumn : public TStorage<std::string> {
public:
    TStringColumn() {}

    Expected<void> Setup(std::vector<std::string> data) override;
};

class UnsupportedErr : public IError {
public:
    std::string Print() const override {
        return "NotAnIntErr";
    }
};

Expected<IColumn> MakeColumn(std::vector<std::string> data, std::string type);
Expected<IColumn> MakeColumnJF(std::vector<std::string> data, std::string type);

} // namespace JFEngine