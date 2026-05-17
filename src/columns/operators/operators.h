#pragma once

#include "../types/types.h"
#include "arithmetic_operators.h"
#include "io_operators.h"

namespace JfEngine {

template <typename TOperator, typename... Args>
auto Do(TColumnPtr col, Args&&... args) {
    switch (col->GetType()) {
        case ki8Column: {
            return TOperator::Exec(*static_cast<Ti8Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case ki16Column: {
            return TOperator::Exec(*static_cast<Ti16Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case ki32Column: {
            return TOperator::Exec(*static_cast<Ti32Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case ki64Column: {
            return TOperator::Exec(*static_cast<Ti64Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case ki128Column: {
            return TOperator::Exec(*static_cast<Ti128Column*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case kDoubleColumn: {
            return TOperator::Exec(*static_cast<TDoubleColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case kDateColumn: {
            return TOperator::Exec(*static_cast<TDateColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case kTimestampColumn: {
            return TOperator::Exec(*static_cast<TTimestampColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
        case kStringColumn: {
            return TOperator::Exec(*static_cast<TStringColumn*>(col.get()), std::forward<Args>(args)...);
            break;
        }
    }
    throw std::runtime_error("bad column type");
}

} // namespace JfEngine