#include "operators.h"

#include <algorithm>

namespace JfEngine {

struct OSort {
    static std::vector<ui64> Exec(TDateColumn& col, bool rev = false) {
        std::vector<ui64> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = i;
        }
        std::stable_sort(ans.begin(), ans.end(), [&col, rev](ui64 i, ui64 j) -> bool {
            if (rev) {
                return (col.GetData()[i].IntDate() > col.GetData()[j].IntDate());
            }
            return (col.GetData()[i].IntDate() < col.GetData()[j].IntDate());
        });
        return std::move(ans);
    }

    static std::vector<ui64> Exec(TTimestampColumn& col, bool rev = false) {
        std::vector<ui64> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = i;
        }
        std::stable_sort(ans.begin(), ans.end(), [&col, rev](ui64 i, ui64 j) -> bool {
            if (rev) {
                return (col.GetData()[i].IntTime() > col.GetData()[j].IntTime());
            }
            return (col.GetData()[i].IntTime() < col.GetData()[j].IntTime());
        });
        return std::move(ans);
    }

    template <typename T>
    static std::vector<ui64> Exec(T& col, bool rev = false) {
        std::vector<ui64> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = i;
        }
        std::stable_sort(ans.begin(), ans.end(), [&col, rev](ui64 i, ui64 j) -> bool {
            if (rev) {
                return (col.GetData()[i] > col.GetData()[j]);
            }
            return (col.GetData()[i] < col.GetData()[j]);
        });
        return std::move(ans);
    }
};

struct OApplyOrder {
    static Expected<IColumn> Exec(Ti8Column& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<i8> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<Ti8Column>(ans);
    }

    static Expected<IColumn> Exec(Ti16Column& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<i16> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<Ti16Column>(ans);
    }

    static Expected<IColumn> Exec(Ti32Column& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<i32> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<Ti32Column>(ans);
    }

    static Expected<IColumn> Exec(Ti64Column& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<i64> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<Ti64Column>(ans);
    }

    static Expected<IColumn> Exec(TDateColumn& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<TDate> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<TDateColumn>(ans);
    }

    static Expected<IColumn> Exec(TTimestampColumn& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<TTimestamp> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<TTimestampColumn>(ans);
    }

    static Expected<IColumn> Exec(TDoubleColumn& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<ld> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<TDoubleColumn>(ans);
    }

    static Expected<IColumn> Exec(TStringColumn& col, const std::vector<ui64>& order) {
        if (order.size() != col.GetSize()) {
            std::cout << "ordering error" << " " << order.size() << " " << col.GetSize() << std::endl;
            return EError::BadArgsErr;
        }
        std::vector<std::string> ans(col.GetSize());
        for (ui64 i = 0; i < col.GetSize(); i++) {
            ans[i] = col.GetData()[order[i]];
        }
        return std::make_shared<TStringColumn>(ans);
    }
};

struct OCmpFirst {
    template<typename T>
    static i8 Exec(T& self, ui64 self_i, TColumnPtr other, ui64 other_i) {
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

    static ui64 Cmp(Ti8Column& self, ui64 self_i, Ti8Column& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }

    static ui64 Cmp(Ti16Column& self, ui64 self_i, Ti16Column& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }

    static ui64 Cmp(Ti32Column& self, ui64 self_i, Ti32Column& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }

    static ui64 Cmp(Ti64Column& self, ui64 self_i, Ti64Column& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }

    static ui64 Cmp(TDateColumn& self, ui64 self_i, TDateColumn& other, ui64 other_i) {
        return 1;
    }

    static ui64 Cmp(TTimestampColumn& self, ui64 self_i, TTimestampColumn& other, ui64 other_i) {
        return 1;
    }

    static ui64 Cmp(TDoubleColumn& self, ui64 self_i, TDoubleColumn& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }

    static ui64 Cmp(TStringColumn& self, ui64 self_i, TStringColumn& other, ui64 other_i) {
        if (self.GetData()[self_i] < other.GetData()[other_i]) {
            return 1;
        } else {
            return -1;
        }
    }
};

} // namespace JfEngine