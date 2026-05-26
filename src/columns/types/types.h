#pragma once

#include "memory/arena.h"
#include "utils/cint/double.h"
#include "utils/cint/int.h"
#include "utils/errors/errors_templates.h"
#include "utils/faster_vectors/vector_1d.h"
#include "utils/faster_vectors/vector_string_2d.h"
#include "utils/compress/bitpack.h"
#include "utils/compress/dict.h"
#include "utils/compress/delta.h"

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
const std::string ki128SchemeAlias = "int128";
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
    ki128Column,
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

    std::vector<T>& GetData() {
        return cols_;
    }

    Expected<void> Setup(std::vector<T> data) {
        cols_ = std::move(data);
        return EError::NoError;
    };
    virtual Expected<void> Setup(std::vector<std::string>&& data) = 0;
    virtual Expected<void> Setup(const TVectorString2d& data, ui64 column_i) = 0;

protected:
    std::vector<T> cols_;
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

class Ti128Column : public TStorage<i128> {
public:
    Ti128Column() {}
    Ti128Column(std::vector<i128> data);

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
private:
    struct from_chars_result_128 {
        const char* ptr;
        std::errc ec;
    };

    static from_chars_result_128 from_chars(const char* first, const char* last, i128& value, int base = 10);
};

class TStringColumn : public TStorage<JString> {
public:
    // using ElemTypeRo = std::string_view;

    TStringColumn() {}
    TStringColumn(std::vector<JString> data);
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

static_assert(sizeof(TDate) == sizeof(ui32));

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

static_assert(sizeof(TTimestamp) == sizeof(ui64));

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

Expected<TColumnPtr> MakeEmptyColumn(EColumn type);
Expected<TColumnPtr> MakeColumn(std::vector<std::string> data, EColumn type);
Expected<TColumnPtr> MakeColumnOptimized(const TVectorString2d& data, ui64 column_i, EColumn type);
Expected<TColumnPtr> MakeColumnJf(std::vector<char> data, EColumn type);

template <typename T>
Expected<TColumnPtr> SetupColumn(std::vector<std::string>&& data) {
    auto res = std::make_shared<T>();
    auto t = res->Setup(std::move(data));
    if (t.HasError()) {
        return t.GetError();
    }
    return res;
}

template <typename T>
Expected<TColumnPtr> SetupColumn(const TVectorString2d& data, ui64 column_i) {
    auto res = std::make_shared<T>();
    auto t = res->Setup(data, column_i);
    if (t.HasError()) {
        return t.GetError();
    }
    return res;
}

template <typename TCol>
concept CIntegralColumn =
       std::same_as<TCol, Ti8Column>
    || std::same_as<TCol, Ti16Column>
    || std::same_as<TCol, Ti32Column>
    || std::same_as<TCol, Ti64Column>
    || std::same_as<TCol, Ti128Column>;

template <typename TCol>
concept CDoubleColumn = std::same_as<TCol, TDoubleColumn>;

template <typename TCol>
concept CStringColumn = std::same_as<TCol, TDoubleColumn>;

template <typename TCol>
concept CTimeColumn =
       std::same_as<TCol, TDateColumn>
    || std::same_as<TCol, TTimestampColumn>;

template <typename T>
inline std::vector<char> Serialize(std::vector<T>& a);

template <std::integral T>
inline std::vector<char> Serialize(std::vector<T>& a) {
    using U = std::make_unsigned_t<T>;
    constexpr ui32 W = sizeof(T) * 8;

    auto encode = [](T v) -> U {
        if constexpr (std::is_signed_v<T>) {
            U u = static_cast<U>(v);
            return (u << 1) ^ static_cast<U>(static_cast<std::make_signed_t<U>>(u) >> (W - 1));
        } else {
            return static_cast<U>(v);
        }
    };

    U max_encoded = 0;
    for (auto v : a) {
        U e = encode(v);
        if (e > max_encoded) max_encoded = e;
    }
    ui8 bits = static_cast<ui8>(std::bit_width(static_cast<ui64>(max_encoded)));

    auto packed = BitPack(a, bits, encode);

    std::vector<char> res(sizeof(bits) + packed.size());
    std::memcpy(res.data(), &bits, sizeof(bits));
    std::memcpy(res.data() + sizeof(bits), packed.data(), packed.size());
    return res;
}

template<>
inline std::vector<char> Serialize<TDate>(std::vector<TDate>& a) {
    return {};
}

template<>
inline std::vector<char> Serialize<TTimestamp>(std::vector<TTimestamp>& a) {
    return {};
}

template<>
inline std::vector<char> Serialize<JString>(std::vector<JString>& a) {
    return DictSerialize(a);
}

template <typename T>
inline std::vector<T> Unserialize(const std::vector<char>& a);

template <typename T>
inline std::vector<T> Unserialize(const std::vector<char>& a) {
    std::vector<T> res(a.size() / sizeof(T));
    std::memcpy(res.data(), a.data(), a.size());
    return res;
}
template <std::integral T>
inline std::vector<T> Unserialize(const std::vector<char>& a) {
    using U = std::make_unsigned_t<T>;

    ui8 bits = static_cast<ui8>(a.at(0));

    auto decode = [](U e) -> T {
        if constexpr (std::is_signed_v<T>) {
            return static_cast<T>((e >> 1) ^ -static_cast<U>(e & 1));
        } else {
            return static_cast<T>(e);
        }
    };

    std::vector<T> res;
    BitUnpack(a.data() + sizeof(bits), a.size() - sizeof(bits), bits, res, decode);
    return res;
}
template<>
inline std::vector<JString> Unserialize<JString>(std::vector<char>& a) {
    return DictUnserialize(a);
}

} // namespace JfEngine
