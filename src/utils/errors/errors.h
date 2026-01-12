#pragma once

#include "utils/cint/int.h"

#include <string>
#include <string_view>
#include <typeinfo>
#include <type_traits>

class IError {
public:
    virtual ~IError() = default;
    virtual std::string Print() const = 0;

    virtual i64 GetId() const;
};

template <typename T>
bool Is(IError* in) {
    if (!in) {
        return false;
    }
    return in->GetId() == typeid(T).hash_code();
}
