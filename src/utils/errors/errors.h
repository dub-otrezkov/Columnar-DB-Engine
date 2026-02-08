#pragma once

#include "utils/cint/int.h"

#include <cassert>
#include <string>
#include <string_view>
#include <typeinfo>
#include <memory>

class IError {
public:
    virtual ~IError() = default;
    virtual std::string Print() const = 0;

    virtual i64 GetId() const;
};

using Error = std::shared_ptr<IError>;

template <typename T>
bool Is(Error in) {
    if (!in) {
        return false;
    }
    return in->GetId() == typeid(T).hash_code();
}

template <typename T, typename... Args>
Error MakeError(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
class Expected {
public:
    Expected(std::nullptr_t) {}

    Expected(T&& res, Error err = nullptr) : res_(std::make_shared<T>(res)), err_(err) {}

    Expected(std::shared_ptr<T> res, Error err = nullptr) : res_(res), err_(err) {}

    template<typename R>
    Expected(std::shared_ptr<R> res, Error err = nullptr) : res_(res), err_(err) {}

    Expected(Error err) : err_(err) {}

    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) = default;

    bool HasError() const {
        return (err_ ? true : false);
    }

    Error GetError() {
        return err_;
    }

    T& GetRes() {
        return *res_;
    }

    auto GetShared() {
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
    Error err_ = nullptr;
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
        using type = Error;
    };
}

template <>
class Expected<void> {
public:
    Expected() {}

    Expected(std::nullptr_t) {}

    Expected(Error err) : err_(err) {}

    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) = default;

    bool HasError() const {
        return (err_ ? true : false);
    }

    explicit operator bool() const {
        return !HasError();
    }

    Error GetError() {
        return err_;
    }
private:
    Error err_ = nullptr;
};
