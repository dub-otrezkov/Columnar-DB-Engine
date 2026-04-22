#pragma once

#include "../types/types.h"

#include <algorithm>

namespace JfEngine {

struct OExtractMinute {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col) {
        return std::make_shared<Ti64Column>(std::vector<i64>(col.GetSize(), 0));
    }

    static inline Expected<IColumn> Exec(TTimestampColumn& col) {
        std::vector<i64> ans(col.GetSize());
        for (i64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData().at(i).minute;
        } 
        return std::make_shared<Ti64Column>(std::move(ans));
    }
};

} // namespace JfEngine