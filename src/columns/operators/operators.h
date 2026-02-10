#pragma once

#include "io_operators.h"
#include "arithmetic_operators.h"

#include "../types/types.h"

namespace JFEngine {

template <typename TOperator, typename... Args>
auto Do(TColumnPtr col, Args&&... args) {
    switch (col->GetType()) {
        case Ei8Column: {
            return TOperator::Exec(*static_cast<Ti8Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case Ei16Column: {
            return TOperator::Exec(*static_cast<Ti16Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case Ei32Column: {
            return TOperator::Exec(*static_cast<Ti32Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case Ei64Column: {
            return TOperator::Exec(*static_cast<Ti64Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EDoubleColumn: {
            return TOperator::Exec(*static_cast<TDoubleColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EDateColumn: {
            return TOperator::Exec(*static_cast<TDateColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case ETimestampColumn: {
            return TOperator::Exec(*static_cast<TTimestampColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case EStringColumn: {
            return TOperator::Exec(*static_cast<TStringColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
    }
    throw std::runtime_error("bad column type");
}

} // namespace JFEngine