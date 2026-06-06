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
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
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
struct TLoaderSlot {
    std::function<std::vector<T>()> fn;
    std::vector<T> data;
    ui64 n = 0;
    bool ready = false;

    std::vector<T>& Get() {
        if (!ready) {
            data = fn();
            ready = true;
        }
        return data;
    }
};

template <typename T>
class TColData {
public:
    TColData() = default;

    TColData(std::vector<T> data) {
        auto slot = std::make_shared<TLoaderSlot<T>>();
        slot->ready = true;
        slot->n = data.size();
        slot->data = std::move(data);
        loaders_.push_back(std::move(slot));
    }

    TColData(ui64 n, std::function<std::vector<T>()> fn) {
        auto slot = std::make_shared<TLoaderSlot<T>>();
        slot->fn = std::move(fn);
        slot->n = n;
        loaders_.push_back(std::move(slot));
    }

    TColData& operator=(std::vector<T> data) {
        *this = TColData(std::move(data));
        return *this;
    }

    ui64 size() const {
        if (!idx_.empty()) {
            return idx_.size();
        }
        return loaders_.empty() ? 0 : loaders_.front()->n;
    }

    bool empty() const {
        return size() == 0;
    }

    T& at(ui64 i) {
        if (idx_.empty()) {
            return loaders_.front()->Get().at(i);
        }
        const auto& p = idx_.at(i);
        return loaders_.at(p.second)->Get().at(p.first);
    }

    const T& at(ui64 i) const {
        if (idx_.empty()) {
            return loaders_.front()->Get().at(i);
        }
        const auto& p = idx_.at(i);
        return loaders_.at(p.second)->Get().at(p.first);
    }

    T& back() {
        return at(size() - 1);
    }

    void reserve(ui64 n) {
        Build().data.reserve(n);
    }

    void push_back(const T& value) {
        auto& slot = Build();
        slot.data.push_back(value);
        slot.n = slot.data.size();
    }

    template <typename... TArgs>
    void emplace_back(TArgs&&... args) {
        auto& slot = Build();
        slot.data.emplace_back(std::forward<TArgs>(args)...);
        slot.n = slot.data.size();
    }

    void resize(ui64 n, const T& value) {
        auto& slot = Build();
        slot.data.resize(n, value);
        slot.n = n;
    }

    void assign(ui64 n, const T& value) {
        *this = TColData();
        auto& slot = Build();
        slot.data.push_back(value);
        slot.n = 1;
        idx_.assign(n, std::make_pair(static_cast<ui32>(0), static_cast<ui32>(0)));
    }

    void LoadFrom(TColData<T>& other, ui64 i) {
        ui32 row;
        ui32 src_li;
        if (other.idx_.empty()) {
            row = static_cast<ui32>(i);
            src_li = 0;
        } else {
            row = other.idx_.at(i).first;
            src_li = other.idx_.at(i).second;
        }
        idx_.emplace_back(row, AdoptLoader(other.loaders_.at(src_li)));
    }

    T* data() {
        return Vec().data();
    }

    std::vector<T>& Vec() {
        if (loaders_.empty()) {
            Build();
        }
        if (idx_.empty()) {
            return loaders_.front()->Get();
        }
        std::vector<T> flat;
        flat.reserve(idx_.size());
        for (ui64 i = 0; i < idx_.size(); i++) {
            flat.push_back(at(i));
        }
        *this = TColData(std::move(flat));
        return loaders_.front()->Get();
    }

private:
    TLoaderSlot<T>& Build() {
        if (loaders_.empty()) {
            auto slot = std::make_shared<TLoaderSlot<T>>();
            slot->ready = true;
            loaders_.push_back(std::move(slot));
        }
        return *loaders_.front();
    }

    ui32 AdoptLoader(const std::shared_ptr<TLoaderSlot<T>>& slot) {
        for (ui32 k = 0; k < loaders_.size(); k++) {
            if (loaders_.at(k) == slot) {
                return k;
            }
        }
        loaders_.push_back(slot);
        return static_cast<ui32>(loaders_.size() - 1);
    }

    std::vector<std::shared_ptr<TLoaderSlot<T>>> loaders_;
    std::vector<std::pair<ui32, ui32>> idx_;
};

template <typename T>
class TStorage : public IColumn {
public:
    using ElemType = T;
    using ElemTypeRo = T;

    TStorage() = default;

    TStorage(i64 n, std::function<TColumnPtr()> func) {
        cols_ = TColData<T>(static_cast<ui64>(n), [func]() -> std::vector<T> {
            auto t = func();
            return std::move(static_cast<TStorage<T>*>(t.get())->cols_.Vec());
        });
    }

    ui64 GetSize() const override {
        return cols_.size();
    }

    TColData<T>& GetData() {
        return cols_;
    }

    Expected<void> Setup(std::vector<T> data) {
        cols_ = std::move(data);
        return EError::NoError;
    };
    virtual Expected<void> Setup(std::vector<std::string>&& data) = 0;
    virtual Expected<void> Setup(const TVectorString2d& data, ui64 column_i) = 0;

    void LoadFrom(TStorage<T>* other, ui64 i) {
        cols_.LoadFrom(other->cols_, i);
    }

protected:
    TColData<T> cols_;
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
