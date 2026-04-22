#include "operators.h"

#include <algorithm>

namespace JfEngine {

struct OSort {
    template <typename TCol>
    static inline std::vector<i64> Exec(TCol& col, bool rev = false) {
        using T = typename TCol::ElemType;
        std::vector<i64> ans(col.GetSize());
        for (i64 i = 0; i < col.GetSize(); i++) {
            ans[i] = i;
        }
        std::stable_sort(ans.begin(), ans.end(), [&col, rev](i64 i, i64 j) -> bool {
            if (rev) {
                return !(col.GetData().at(i) <= col.GetData().at(j));
            }
            return (col.GetData().at(i) < col.GetData().at(j));
        });
        return std::move(ans);
    }
};

struct OMergeSort {
    static constexpr ui64 kUnlimited = -1;
    template <typename TCol>
    static inline Expected<std::vector<i64>> Exec(TCol& col, TColumnPtr other, ui64 limit = kUnlimited, bool rev = false) {
        using T = typename TCol::ElemType;
        if (col.GetType() != other->GetType()) {
            return MakeError<EError::BadArgsErr>("cant merge columns of different type");
        }
        auto& col2 = *static_cast<TCol*>(other.get());

        auto cmp = [&col, &col2, rev](const i64& i, const i64& j) -> bool {
            if (rev) {
                return !(col.GetData().at(i) <= col2.GetData().at(j));
            }
            return (col.GetData().at(i) < col2.GetData().at(j));
        };

        i64 i = 0;
        i64 j = 0;
        auto& data1 = col.GetData();
        auto& data2 = col2.GetData();

        auto order = OSort::Exec(col2, rev);
        std::vector<i64> ans;

        if (limit == kUnlimited) {
            ans.reserve(data1.size() + data2.size());
        } else {
            ans.reserve(limit);
        }

        while (i < data1.size() || j < data2.size()) {
            if (i == data1.size()) {
                ans.push_back(-(order[j] + 1));
                j++;
            } else if (j == data2.size()) {
                ans.push_back(i);
                i++;
            } else if (cmp(i, order[j])) {
                ans.push_back(i);
                i++;
            } else {
                ans.push_back(-(order[j] + 1));
                j++;
            }

            if (ans.size() == limit) {
                break;
            }
        }
        return std::move(ans);
    }
};

struct OApply2 {
    template <typename TCol>
    static inline Expected<IColumn> Exec(TCol& col, TColumnPtr other, const std::vector<i64>& ids) {
        using T = typename TCol::ElemType;
        if (col.GetType() != other->GetType()) {
            return MakeError<EError::BadArgsErr>("cant merge different columns");
        }
        FlatVector<T> ans;
        ans.reserve(ids.size());

        auto& data1 = col.GetData();
        auto& data2 = static_cast<TCol*>(other.get())->GetData();
        for (auto& i : ids) {
            if (i >= 0) {
                ans.push_back(data1.at(i));
            } else {
                // std::cout << -i - 1 << std::endl;
                // if constexpr (std::is_same_v<T, i64>) {
                //     std::cout << "------ " << data2.at(-i - 1) << std::endl;
                // }
                ans.push_back(data2.at(-i - 1));
            }
        }

        return std::make_shared<TCol>(std::move(ans));
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
            ans[i] = col.GetData().at(order[i]);
        }
        return std::make_shared<TCol>(std::move(ans));
    }

    static inline Expected<IColumn> Exec(TStringColumn& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            return EError::BadArgsErr;
        }
        StringVector ans;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans.push_back_mcpy(col.GetData().data() + col.GetData().get_pos(order[i]), col.GetData().get_len(order[i]));
        }
        return std::make_shared<TStringColumn>(std::move(ans));
    }
};

struct OCmp {
    template <typename TCol>
    static inline i64 Exec(TCol& self, i64 i1, i64 i2) {
        if (self.GetData().at(i1) < self.GetData().at(i2)) {
            return 1;
        } else if (self.GetData().at(i1) == self.GetData().at(i2)) {
            return 0;
        } else {
            return -1;
        }
    }
};

struct OCmpDiffCol {
    template <typename TCol>
    static inline i64 Exec(TCol& self, TColumnPtr other, i64 i1, i64 i2) {
        if (self.GetType() != other->GetType()) {
            return 0;
        }
        auto& ot = *static_cast<TCol*>(other.get());
        if (self.GetData().at(i1) < ot.GetData().at(i2)) {
            return 1;
        } else if (self.GetData().at(i1) == ot.GetData().at(i2)) {
            return 0;
        } else {
            return -1;
        }
    }
};

} // namespace JfEngine