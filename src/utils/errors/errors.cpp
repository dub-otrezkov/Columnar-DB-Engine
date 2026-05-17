#include "errors.h"

i64 IError::GetId() const {
    return typeid(*this).hash_code();
}