#pragma once

#include "utils/cint/int.h"

#include <cassert>
#include <string>
#include <string_view>
#include <typeinfo>
#include <memory>
#include <optional>

enum EError {
    NoError = 0,
    NotAnIntErr,
    NotAnDateErr,
    NotAnTimestampErr,
    IntOverflowErr,
    IONotFoundErr,
    BadCmdErr,
    BadArgsErr,
    UnsupportedErr,
    EofErr,
    IncorrectFileErr,
    UnimplementedErr,
    NoSuchColumnsErr
};

class IError {
public:
    virtual ~IError() = default;
    virtual std::string Print() const = 0;

    virtual i64 GetId() const;
};

// using Error = std::shared_ptr<IError>;

template <EError T>
bool Is(EError in) {
    // if (!in) {
    //     return false;
    // }
    // return in->GetId() == typeid(T).hash_code();
    return T == in;
}

template <EError T, typename... Args>
EError MakeError(Args&&... args) {
    // return std::make_shared<T>(std::forward<Args>(args)...);
    return T;
}

template <typename T>
class Expected {
public:
    Expected(std::nullptr_t) {}

    Expected(T&& res, EError err = EError::NoError) : res_(std::make_shared<T>(std::forward<T>(res))), err_(err) {}

    Expected(std::shared_ptr<T> res, EError err = EError::NoError) : res_(std::move(res)), err_(err) {}

    template<typename R>
    Expected(std::shared_ptr<R> res, EError err = EError::NoError) : res_(std::move(res)), err_(err) {}

    Expected(EError err) : err_(err) {}

    Expected(const Expected&) = delete;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = delete;
    Expected& operator=(Expected&&) = default;

    bool HasError() const {
        return (err_ != EError::NoError);
    }

    EError GetError() {
        return err_;
    }

    T& GetRes() {
        return *res_;
    }

    std::shared_ptr<T> GetShared() {
        return res_;
    }

    explicit operator bool() const {
        return !HasError();
    }

    T* operator->() {
        assert(!HasError());
        return res_;
    }

    auto operator*() {
        return res_;
    }

    template<std::size_t N>
    auto get() const {
        if constexpr (N == 0) {
            return res_;
        }
        else if constexpr (N == 1) {
            return err_;
        }
    }
private:
    std::shared_ptr<T> res_ = nullptr;
    EError err_ = EError::NoError;
};

namespace std {
    template<typename T>
    struct tuple_size<Expected<T>> : integral_constant<size_t, 2> {};

    template<typename T>
    struct tuple_element<0, Expected<T>> {
        using type = std::shared_ptr<T>;
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

    bool HasError() const {
        return (err_ != EError::NoError);
    }

    explicit operator bool() const {
        return !HasError();
    }

    EError GetError() {
        return err_;
    }
private:
    EError err_ = EError::NoError;
};
