#pragma once

#include "utils/errors/errors_templates.h"
#include "utils/cint/int.h"
#include "utils/cint/double.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace JFEngine {

// basic classes

const std::string ki8SchemeAlias = "int8";
const std::string ki16SchemeAlias = "int16";
const std::string ki32SchemeAlias = "int32";
const std::string ki64SchemeAlias = "int64";
const std::string kStringSchemeAlias = "string";
const std::string kDoubleSchemeAlias = "double";
const std::string kDateSchemeAlias = "date";
const std::string kTimestampSchemeAlias = "timestamp";
const std::string kUnknownSchemeAlias = "unknown";

enum TColumn {
    EUnitialized,
    Ei8Column,
    Ei16Column,
    Ei32Column,
    Ei64Column,
    EStringColumn,
    EDoubleColumn,
    EDateColumn,
    ETimestampColumn,
};

TColumn StrToTColumn(const std::string& data);
std::string TColumnToStr(TColumn data);

class IColumn {
public:
    virtual ~IColumn() = default;

    virtual ui64 GetSize() = 0;

    virtual TColumn GetType() {
        return EUnitialized;
    }
};

using TColumnPtr = std::shared_ptr<IColumn>;

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

// colums

class Ti8Column : public TStorage<i8> {
public:
    Ti8Column() {}

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

class Ti16Column : public TStorage<i16> {
public:
    Ti16Column() {}

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

class Ti32Column : public TStorage<i32> {
public:
    Ti32Column() {}

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

class Ti64Column : public TStorage<i64> {
public:
    Ti64Column() {}
    Ti64Column(std::vector<i64> data);

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

class TStringColumn : public TStorage<std::string> {
public:
    TStringColumn() {}
    TStringColumn(std::vector<std::string> data);

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

class TDoubleColumn : public TStorage<ld> {
public:
    TDoubleColumn() {}
    TDoubleColumn(std::vector<ld> data);

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

struct TDate {
    i16 year;
    i8 month;
    i8 day;
};

class TDateColumn : public TStorage<TDate> {
public:
    TDateColumn() {}

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

std::string PrintDate(const TDate& d);

struct TTimestamp {
    TDate date;
    i8 hour;
    i8 minute;
    i8 second;
};

std::string PrintTimestamp(const TTimestamp& d);

class TTimestampColumn : public TStorage<TTimestamp> {
public:
    TTimestampColumn() {}

    TColumn GetType() override;
    Expected<void> Setup(std::vector<std::string> data) override;
};

// helpers

Expected<IColumn> MakeColumn(std::vector<std::string> data, TColumn type);
Expected<IColumn> MakeColumnJF(std::vector<std::string> data, TColumn type);

template <typename T>
Expected<IColumn> SetupColumn(std::vector<std::string>&& data) {
    auto res = std::make_shared<T>();
    auto t = res->Setup(std::move(data));
    if (t.HasError()) {
        return t.GetError();
    }
    return res;
}

} // namespace JFEngine
