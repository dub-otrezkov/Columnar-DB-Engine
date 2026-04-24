#pragma once

#include "utils/cint/int.h"

#include <cassert>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <typeinfo>

enum EError {
    NoError = 0,
    NotAnIntErr,
    NotAnDateErr,
    NotAnTimestampErr,
    IntOverflowErr,
    IoNotFoundErr,
    BadCmdErr,
    BadArgsErr,
    UnsupportedErr,
    EofErr,
    IncorrectFileErr,
    UnimplementedErr,
    NoSuchColumnsErr,
    OutOfRangeErr
};

class IError {
public:
    virtual ~IError() = default;
    virtual std::string Print() const = 0;

    virtual i64 GetId() const;
};

template <EError T>
bool Is(EError in) {
    return T == in;
}

template <EError T>
EError MakeError(const std::string& arg = "") {
    if (!arg.empty()) {
        // std::cout << "GOT ERR: " << " " << T << " " << arg << std::endl;
    }
    return T;
}

// ---------------------------------------------------------------------------
// Expected<T>  —  stores T directly via std::optional<T>
// ---------------------------------------------------------------------------
template <typename T>
class Expected {
    std::optional<T> val_;
    EError err_ = EError::NoError;

public:
    Expected() {}

    Expected(T val, EError err = EError::NoError)
        : val_(std::move(val)), err_(err) {}

    template<typename U>
    Expected(U val, EError err = EError::NoError)
        requires (std::is_constructible_v<T, U&&>
                  && !std::is_same_v<std::decay_t<U>, T>
                  && !std::is_same_v<std::decay_t<U>, EError>
                  && !std::is_same_v<std::decay_t<U>, std::nullptr_t>)
        : val_(T(std::move(val))), err_(err) {}

    Expected(EError err) : err_(err) {}

    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) = default;

    bool HasError() const { return err_ != EError::NoError; }
    bool HasValue() const { return val_.has_value(); }
    EError GetError() const { return err_; }

    T& GetRes() {
        assert(val_.has_value());
        return *val_;
    }

    const T& GetRes() const {
        assert(val_.has_value());
        return *val_;
    }

    explicit operator bool() const { return !HasError(); }

    T* operator->() {
        assert(val_.has_value());
        return &*val_;
    }

    template<std::size_t N>
    auto get() {
        if constexpr (N == 0) {
            if (val_) return std::move(*val_);
            return T{};
        } else {
            return err_;
        }
    }
};

namespace std {
    template<typename T>
    struct tuple_size<Expected<T>> : integral_constant<size_t, 2> {};

    template<typename T>
    struct tuple_element<0, Expected<T>> {
        using type = T;
    };

    template<typename T>
    struct tuple_element<1, Expected<T>> {
        using type = EError;
    };
}

template <>
class Expected<void> {
public:
    Expected() {}

    Expected(std::nullptr_t) {}

    Expected(EError err) : err_(err) {}

    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) = default;

    bool HasError() const { return err_ != EError::NoError; }

    explicit operator bool() const { return !HasError(); }

    EError GetError() { return err_; }

private:
    EError err_ = EError::NoError;
};
