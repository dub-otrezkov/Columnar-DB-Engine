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
EError MakeError(std::string arg = "") {
    if (!arg.empty()) {
        // std::cout << "GOT ERR: " << " " << T << " " << arg << std::endl;
    }
    return T;
}

// ---------------------------------------------------------------------------
// Ownership policy trait — specialize to opt into shared_ptr for Expected<T>
// ---------------------------------------------------------------------------
template <typename T>
struct TExpectedUseShared : std::false_type {};

// Forward-declare JfEngine types that need shared_ptr
namespace JfEngine {
    class IColumn;
    class ITableInput;
}

template <> struct TExpectedUseShared<JfEngine::IColumn>     : std::true_type {};
template <> struct TExpectedUseShared<JfEngine::ITableInput> : std::true_type {};
template <> struct TExpectedUseShared<std::iostream>         : std::true_type {};

// ---------------------------------------------------------------------------
// Expected<T>
// ---------------------------------------------------------------------------
template <typename T>
class Expected {
    static constexpr bool kShared = TExpectedUseShared<T>::value;
    using PtrType = std::conditional_t<kShared, std::shared_ptr<T>, std::unique_ptr<T>>;

    static PtrType MakePtr(T&& val) {
        if constexpr (kShared) {
            return std::make_shared<T>(std::move(val));
        } else {
            return std::make_unique<T>(std::move(val));
        }
    }

public:
    Expected(std::nullptr_t) {}

    Expected(T&& res, EError err = EError::NoError)
        : res_(MakePtr(std::move(res))), err_(err) {}

    Expected(PtrType res, EError err = EError::NoError)
        : res_(std::move(res)), err_(err) {}

    // Derived-type constructor — only meaningful for shared_ptr variant
    template<typename R>
    Expected(std::shared_ptr<R> res, EError err = EError::NoError)
        requires (kShared)
        : res_(std::move(res)), err_(err) {}

    Expected(EError err) : err_(err) {}

    // Copy: implicitly deleted when PtrType = unique_ptr, works for shared_ptr
    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) = default;

    bool HasError() const { return err_ != EError::NoError; }
    EError GetError() { return err_; }
    T& GetRes() { return *res_; }

    // GetShared() — shared_ptr variant only
    std::shared_ptr<T> GetShared() requires (kShared) { return res_; }

    // Release() — unique_ptr variant only
    std::unique_ptr<T> Release() requires (!kShared) { return std::move(res_); }

    explicit operator bool() const { return !HasError(); }

    T* operator->() {
        assert(!HasError());
        return res_.get();
    }

    auto operator*() {
        if constexpr (kShared) {
            return std::make_pair(res_, err_);
        } else {
            return std::make_pair(std::move(res_), err_);
        }
    }

    template<std::size_t N>
    auto get() {
        if constexpr (N == 0) {
            if constexpr (kShared) {
                return res_;
            } else {
                return std::move(res_);
            }
        } else if constexpr (N == 1) {
            return err_;
        }
    }

private:
    PtrType res_ = nullptr;
    EError err_ = EError::NoError;
};

namespace std {
    template<typename T>
    struct tuple_size<Expected<T>> : integral_constant<size_t, 2> {};

    template<typename T>
    struct tuple_element<0, Expected<T>> {
        using type = conditional_t<
            TExpectedUseShared<T>::value,
            shared_ptr<T>,
            unique_ptr<T>
        >;
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
