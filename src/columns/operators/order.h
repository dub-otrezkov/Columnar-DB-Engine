#include "operators.h"

#include <algorithm>

namespace JfEngine {

struct OSort {
    template <typename TCol>
    static inline std::vector<ui64> Exec(TCol& col, bool rev = false) {
        using T = typename TCol::ElemType;
        std::vector<ui64> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = i;
        }
        std::stable_sort(ans.begin(), ans.end(), [&col, rev](ui64 i, ui64 j) -> bool {
            if (rev) {
                return !(col.GetData()[i] <= col.GetData()[j]);
            }
            return (col.GetData()[i] < col.GetData()[j]);
        });
        return std::move(ans);
    }
};

struct OApplyOrder {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col, const std::vector<ui64>& order) {
        using T = typename TCol::ElemType;
        if (order.size() != col.GetSize()) {
            return EError::BadArgsErr;
        }
        std::vector<T> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<TCol>(ans);
    }
};

struct OCmpFirst {
    template<typename T>
    static inline i8 Exec(T& self, ui64 self_i, TColumnPtr other, ui64 other_i) {
        if (self_i == self.GetSize()) {
            return 1;
        }
        if (other_i == other->GetSize()) {
            return -1;
        }
        if (other->GetType() != self.GetType()) {
            throw "wtf";
        }
        return Cmp(self, self_i, *static_cast<T*>(other.get()));
    }

    template <typename T>
    static inline ui64 Cmp(T& self, ui64 self_i, T& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }
};

} // namespace JfEngine