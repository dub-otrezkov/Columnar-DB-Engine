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

#include <bit>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace JfEngine {

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
    
    TStorage() = default;

    TStorage(i64 n, std::function<TColumnPtr()> func) {
        auto src = std::make_shared<std::function<TColumnPtr()>>(std::move(func));
        load_data_ = std::vector<std::function<void(std::vector<T>&)>>{};
        load_data_->reserve(n);
        for (i64 i = 0; i < n; i++) {
            load_data_->emplace_back([src, i](std::vector<T>& out) -> void {
                auto t = (*src)();
                out.push_back(static_cast<TStorage<T>*>(t.get())->cols_.at(i));
            });
        }
    }

    ui64 GetSize() const override {
        return (load_data_.has_value() ? load_data_->size() : cols_.size());
    }

    std::vector<T>& GetData() {
        Materialize();
        return cols_;
    }

    Expected<void> Setup(std::vector<T> data) {
        cols_ = std::move(data);
        return EError::NoError;
    };
    virtual Expected<void> Setup(std::vector<std::string>&& data) = 0;
    virtual Expected<void> Setup(const TVectorString2d& data, ui64 column_i) = 0;

    void Materialize() {
        if (load_data_) {
            auto loaders = std::move(*load_data_);
            load_data_ = std::nullopt;
            for (auto& f : loaders) {
                f(cols_);
            }
        }
    }
    
    void LoadFrom(TStorage<T>* other, ui64 i) {
        if (other->load_data_) {
            if (!load_data_) {
                load_data_ = std::vector<std::function<void(std::vector<T>&)>>{};
            }
            load_data_->push_back(other->load_data_->at(i));
        } else {
            cols_.push_back(other->cols_.at(i));
        }
    }

protected:
    std::vector<T> cols_;

    std::optional<std::vector<std::function<void(std::vector<T>&)>>> load_data_;
};

class Ti8Column : public TStorage<i8> {
public:
    Ti8Column() {}
    Ti8Column(std::vector<i8> data);
    Ti8Column(i64 n, std::function<TColumnPtr()> func) : TStorage<i8>(n, func) {}
    
    // Ti8Column() {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti16Column : public TStorage<i16> {
public:
    Ti16Column() {}
    Ti16Column(std::vector<i16> data);
    Ti16Column(i64 n, std::function<TColumnPtr()> func) : TStorage<i16>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti32Column : public TStorage<i32> {
public:
    Ti32Column() {}
    Ti32Column(std::vector<i32> data);
    Ti32Column(i64 n, std::function<TColumnPtr()> func) : TStorage<i32>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti64Column : public TStorage<i64> {
public:
    Ti64Column() {}
    Ti64Column(std::vector<i64> data);
    Ti64Column(i64 n, std::function<TColumnPtr()> func) : TStorage<i64>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class Ti128Column : public TStorage<i128> {
public:
    Ti128Column() {}
    Ti128Column(std::vector<i128> data);
    Ti128Column(i64 n, std::function<TColumnPtr()> func) : TStorage<i128>(n, func) {}

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
    TStringColumn() {}
    TStringColumn(std::vector<JString> data);
    TStringColumn(i64 n, std::function<TColumnPtr()> func) : TStorage<JString>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

class TDoubleColumn : public TStorage<ld> {
public:
    TDoubleColumn() {};
    TDoubleColumn(std::vector<ld> data);
    TDoubleColumn(i64 n, std::function<TColumnPtr()> func) : TStorage<ld>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

struct alignas(4) TDate {
    ui8 day;
    ui8 month;
    ui16 year;

    TDate() = default;

    TDate(ui16 y, ui8 m, ui8 d) : year(y), month(m), day(d) {
    }

    inline ui64 IntDate() const {
        return std::bit_cast<ui32>(*this);
    }

    inline auto operator==(const TDate& other) const {
        return IntDate() == other.IntDate();
    }

    inline auto operator<=>(const TDate& other) const {
        return IntDate() <=> other.IntDate();
    }
};

static_assert(sizeof(TDate) == sizeof(ui32));
static_assert(alignof(TDate) == alignof(ui32));

class TDateColumn : public TStorage<TDate> {
public:
    TDateColumn() {}
    TDateColumn(std::vector<TDate> data);
    TDateColumn(i64 n, std::function<TColumnPtr()> func) : TStorage<TDate>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

std::string PrintDate(const TDate& d);
TDate DateFromStr(const std::string& s);

struct alignas(8) TTimestamp {
    ui8 second;
    ui8 minute;
    ui16 hour;
    TDate date;

    TTimestamp() = default;

    TTimestamp(TDate d, ui8 h, ui8 m, ui8 s) : date(d), hour(h), minute(m), second(s) {
    }

    inline ui64 IntTime() const {
        return std::bit_cast<ui64>(*this);
    }

    inline auto operator==(const TTimestamp& other) const {
        return IntTime() == other.IntTime();
    }

    inline auto operator<=>(const TTimestamp& other) const {
        return IntTime() <=> other.IntTime();
    }
};

static_assert(sizeof(TTimestamp) == sizeof(ui64));
static_assert(alignof(TTimestamp) == alignof(ui64));

std::string PrintTimestamp(const TTimestamp& d);
TTimestamp TimestampFromStr(const std::string& s);

class TTimestampColumn : public TStorage<TTimestamp> {
public:
    TTimestampColumn() {}
    TTimestampColumn(std::vector<TTimestamp> data);
    TTimestampColumn(i64 n, std::function<TColumnPtr()> func) : TStorage<TTimestamp>(n, func) {}

    EColumn GetType() const override;
    Expected<void> Setup(std::vector<std::string>&& data) override;
    Expected<void> Setup(const TVectorString2d& data, ui64 column_i) override;
};

Expected<TColumnPtr> MakeEmptyColumn(EColumn type);
Expected<TColumnPtr> MakeColumn(std::vector<std::string> data, EColumn type);
Expected<TColumnPtr> MakeColumnOptimized(const TVectorString2d& data, ui64 column_i, EColumn type);
Expected<TColumnPtr> MakeColumnJf(std::span<const char> data, EColumn type);
Expected<TColumnPtr> MakeColumnLazy(ui64 n, std::function<TColumnPtr()> data, EColumn type);

Expected<TColumnPtr> ExtractMinMax(std::span<const char> data, EColumn type);

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
concept CTimeColumn =
       std::same_as<TCol, TDateColumn>
    || std::same_as<TCol, TTimestampColumn>;

template <typename T>
inline std::vector<char> Serialize(std::vector<T>& a) {
    throw "bad type";
}

template <typename T>
concept CHasMinMax =
        CIntegralColumn<T> || CTimeColumn<T>;

template <std::integral T>
inline std::vector<char> Serialize(std::vector<T>& a) {
    T mn{};
    T mx{};
    if (!a.empty()) {
        auto [mn_it, mx_it] = std::minmax_element(a.begin(), a.end());
        mn = *mn_it;
        mx = *mx_it;
    }
    auto packed = BitPack(a.size(), a.data());
    if (!a.empty()) {
        auto old = packed.size();
        packed.resize(old + 2 * sizeof(T));
        std::memcpy(packed.data() + old,              &mn, sizeof(T));
        std::memcpy(packed.data() + old + sizeof(T),  &mx, sizeof(T));
    }
    return packed;
}

template<>
inline std::vector<char> Serialize<TDate>(std::vector<TDate>& a) {
    TDate mn{};
    TDate mx{};
    if (!a.empty()) {
        auto [mn_it, mx_it] = std::minmax_element(a.begin(), a.end());
        mn = *mn_it;
        mx = *mx_it;
    }
    auto packed = DeltaSerialize<ui32>(a.size(), reinterpret_cast<ui32*>(a.data()));
    if (!a.empty()) {
        auto old = packed.size();
        packed.resize(old + 2 * sizeof(TDate));
        std::memcpy(packed.data() + old,                  &mn, sizeof(TDate));
        std::memcpy(packed.data() + old + sizeof(TDate),  &mx, sizeof(TDate));
    }
    return packed;
}

template<>
inline std::vector<char> Serialize<TTimestamp>(std::vector<TTimestamp>& a) {
    TTimestamp mn{};
    TTimestamp mx{};
    if (!a.empty()) {
        auto [mn_it, mx_it] = std::minmax_element(a.begin(), a.end());
        mn = *mn_it;
        mx = *mx_it;
    }
    auto packed = DeltaSerialize<ui64>(a.size(), reinterpret_cast<ui64*>(a.data()));
    if (!a.empty()) {
        auto old = packed.size();
        packed.resize(old + 2 * sizeof(TTimestamp));
        std::memcpy(packed.data() + old,                       &mn, sizeof(TTimestamp));
        std::memcpy(packed.data() + old + sizeof(TTimestamp),  &mx, sizeof(TTimestamp));
    }
    return packed;
}

template<>
inline std::vector<char> Serialize<JString>(std::vector<JString>& a) {
    return DictSerialize(a.size(), a.data());
}

template <>
inline std::vector<char> Serialize<ld>(std::vector<ld>& a) {
    ld mn{};
    ld mx{};
    if (!a.empty()) {
        auto [mn_it, mx_it] = std::minmax_element(a.begin(), a.end());
        mn = *mn_it;
        mx = *mx_it;
    }
    std::vector<char> packed(a.size() * sizeof(ld));
    std::memcpy(packed.data(), a.data(), packed.size());
    if (!a.empty()) {
        auto old = packed.size();
        packed.resize(old + 2 * sizeof(ld));
        std::memcpy(packed.data() + old,             &mn, sizeof(ld));
        std::memcpy(packed.data() + old + sizeof(ld), &mx, sizeof(ld));
    }
    return packed;
}

template <typename T>
inline std::vector<T> Unserialize(std::span<const char> a) {
    throw "bad type";
}

template <std::integral T>
inline std::vector<T> Unserialize(std::span<const char> a) {
    std::vector<T> res;
    const ui64 minmax_size = 2 * sizeof(T);
    auto payload = a.size() >= minmax_size ? a.first(a.size() - minmax_size) : a;
    BitUnpack(payload.size(), payload.data(), res);
    return res;
}

template<>
inline std::vector<JString> Unserialize<JString>(std::span<const char> a) {
    return DictUnserialize(a.size(), a.data());
}

template<>
inline std::vector<TDate> Unserialize<TDate>(std::span<const char> a) {
    const ui64 minmax_size = 2 * sizeof(TDate);
    auto payload = a.size() >= minmax_size ? a.first(a.size() - minmax_size) : a;
    auto t = DeltaUnserialize<ui32>(payload.size(), payload.data());
    std::vector<TDate> ans(t.size());
    std::memcpy(ans.data(), t.data(), t.size() * sizeof(ui32));
    return ans;
}

template<>
inline std::vector<TTimestamp> Unserialize<TTimestamp>(std::span<const char> a) {
    const ui64 minmax_size = 2 * sizeof(TTimestamp);
    auto payload = a.size() >= minmax_size ? a.first(a.size() - minmax_size) : a;
    auto t = DeltaUnserialize<ui64>(payload.size(), payload.data());
    std::vector<TTimestamp> ans(t.size());
    std::memcpy(ans.data(), t.data(), t.size() * sizeof(ui64));
    return ans;
}

template<>
inline std::vector<ld> Unserialize<ld>(std::span<const char> a) {
    const ui64 minmax_size = 2 * sizeof(ld);
    auto payload = a.size() >= minmax_size ? a.first(a.size() - minmax_size) : a;
    std::vector<ld> res(payload.size() / sizeof(ld));
    std::memcpy(res.data(), payload.data(), payload.size());
    return res;
}

}
