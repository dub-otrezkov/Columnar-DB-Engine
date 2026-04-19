#pragma once

#include "memory/arena.h"

#include "utils/errors/errors_templates.h"
#include "utils/faster_vectors/vector_string_2d.h"
#include "utils/faster_vectors/vector_1d.h"
#include "utils/cint/int.h"
#include "utils/cint/double.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace JfEngine {

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

enum EColumn {
    kUnitialized = 0,
    ki8Column,
    ki16Column,
    ki32Column,
    ki64Column,
    kStringColumn,
    kDoubleColumn,
    kDateColumn,
    kTimestampColumn,
};

EColumn StrToTColumn(std::string_view data);
std::string TColumnToStr(EColumn data);

class IColumn {
public:
    virtual ~IColumn() = default;

    virtual ui64 GetSize() const = 0;

    virtual EColumn GetType() const {
        return kUnitialized;
    }
};

using TColumnPtr = std::shared_ptr<IColumn>;

template <typename T>
class TStorage : public IColumn {
public:
    using ElemType = T;
    using ElemTypeRo = T;

    ui64 GetSize() const override {
        return cols_.size();
    }

    FlatVector<T>& GetData() {
        return cols_;
    }

    Expected<void> Setup(FlatVector<T> data) {
        cols_ = std::move(data);
        return EError::NoError;
    };
    virtual Expected<void> Setup(std::vector<std::string>&& data) = 0;
    virtual Expected<void> Setup(const TVectorString2d& data, ui64 column_i) = 0;

protected:
    FlatVector<T> cols_;
};

// colums

class Ti8Column : public TStorage<i8> {
public:
    Ti8Column() {}
    Ti8Column(std::vector<i8> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti16Column : public TStorage<i16> {
public:
    Ti16Column() {}
    Ti16Column(std::vector<i16> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti32Column : public TStorage<i32> {
public:
    Ti32Column() {}
    Ti32Column(std::vector<i32> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti64Column : public TStorage<i64> {
public:
    Ti64Column() {}
    Ti64Column(std::vector<i64> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class TStringColumn : public TStorage<std::string> {
public:
    // using ElemTypeRo = std::string_view;

    TStringColumn() {}
    TStringColumn(StringVector data);
    TStringColumn(std::vector<std::string> data);
    TStringColumn(std::vector<std::string_view> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class TDoubleColumn : public TStorage<ld> {
public:
    TDoubleColumn() {};
    TDoubleColumn(std::vector<ld> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

struct TDate {
    i16 year;
    i8 month;
    i8 day;

    inline i64 IntDate() const {
        return (static_cast<i64>(year) << 16) |
               (static_cast<i64>(month) << 8) |
               (static_cast<i64>(day));
    }

    inline bool operator< (const TDate& other) const {
        return IntDate() < other.IntDate();
    }

    inline bool operator== (const TDate& other) const {
        return IntDate() == other.IntDate();
    }

    inline bool operator<= (const TDate& other) const {
        return IntDate() <= other.IntDate();
    }
};

class TDateColumn : public TStorage<TDate> {
public:
    TDateColumn() {}
    TDateColumn(std::vector<TDate> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

std::string PrintDate(const TDate& d);
TDate DateFromStr(const std::string& s);

struct TTimestamp {
    TDate date;
    i8 hour;
    i8 minute;
    i8 second;

    inline i64 IntTime() const {
        return (date.IntDate() << 24) |
               (static_cast<i64>(hour) << 16) |
               (static_cast<i64>(minute) << 8) |
               (static_cast<i64>(second));
    }

    inline bool operator< (const TTimestamp& other) const {
        return IntTime() < other.IntTime();
    }

    inline bool operator== (const TTimestamp& other) const {
        return IntTime() == other.IntTime();
    }

    inline bool operator<= (const TTimestamp& other) const {
        return IntTime() <= other.IntTime();
    }
};

std::string PrintTimestamp(const TTimestamp& d);
TTimestamp TimestampFromStr(const std::string& s);

class TTimestampColumn : public TStorage<TTimestamp> {
public:
    TTimestampColumn() {}
    TTimestampColumn(std::vector<TTimestamp> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

// helpers

Expected<IColumn> MakeEmptyColumn(EColumn type);
Expected<IColumn> MakeColumn(std::vector<std::string> data, EColumn type);
Expected<IColumn> MakeColumnOptimized(const TVectorString2d& data, ui64 column_i, EColumn type);
Expected<IColumn> MakeColumnJf(std::vector<char> data, EColumn type);

template <typename T>
Expected<IColumn> SetupColumn(std::vector<std::string>&& data) {
    auto res = std::allocate_shared<T>(ArenaAlloc());
    auto t = res->Setup(std::move(data));
    if (t.HasError()) {
        return t.GetError();
    }
    return res;
}

template <typename T>
Expected<IColumn> SetupColumn(const TVectorString2d& data, ui64 column_i) {
    auto res = std::allocate_shared<T>(ArenaAlloc());
    auto t = res->Setup(data, column_i);
    if (t.HasError()) {
        return t.GetError();
    }
    return res;
}

} // namespace JfEngine
