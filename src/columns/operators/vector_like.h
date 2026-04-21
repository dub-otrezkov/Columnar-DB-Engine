#include "operators.h"

#include "../types/types.h"

namespace JfEngine {

struct OPushBack {
    template <typename TCol>
    static inline void Exec(TCol& col, typename TCol::ElemTypeRo value) {
        col.GetData().push_back(value);
    }
};

// from, to, i
struct OPushBackFrom {
    template<typename TCol>
    static inline void Exec(TCol& from, TColumnPtr to, i64 i) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        OPushBack::Exec(*static_cast<TCol*>(to.get()), from.GetData()[i]);
    }
};

struct OPushBackFromBatch {
    template<typename TCol>
    static inline void Exec(TCol& from, TColumnPtr to, std::vector<ui64>& is) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        auto& t = *static_cast<TCol*>(to.get());
        t.GetData().reserve(t.GetData().size() + is.size());
        for (const auto& i : is) {
            assert(i < from.GetData().size());
            OPushBack::Exec(t, from.GetData()[i]);
        }
    }
};

// from, to
struct OPushBackVector {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& from, TColumnPtr to) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        auto target = static_cast<TCol*>(to.get());
        ui64 prev_sz = target->GetData().size();
        target->GetData().resize(from.GetData().size() + target->GetData().size());
        std::memcpy(
            reinterpret_cast<char*>(target->GetData().data() + prev_sz),
            reinterpret_cast<char*>(from.GetData().data()),
            from.GetData().size() * sizeof(typename TCol::ElemType)
        );
        return EError::NoError;
    }

    static inline Expected<void> Exec(TStringColumn& from, TColumnPtr to) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        auto target = static_cast<TStringColumn*>(to.get());
        ui64 dl = target->GetData().data_size();
        ui64 prev_sz = target->GetData().size();
        target->GetData().resize_both(
            from.GetData().data_size() + target->GetData().data_size(),
            from.GetData().size() + target->GetData().size()
        );
        std::memcpy(
            reinterpret_cast<char*>(target->GetData().data() + dl),
            reinterpret_cast<char*>(from.GetData().data()),
            from.GetData().data_size()
        );
        std::memcpy(
            reinterpret_cast<char*>(target->GetData().offsets_data() + prev_sz),
            reinterpret_cast<char*>(from.GetData().offsets_data()),
            from.GetData().size() * sizeof(ui64)
        );
        for (ui64 i = 0; i < from.GetData().size(); i++) {
            target->GetData().offsets_data()[prev_sz + i] += dl;
        }
        return EError::NoError;
    }
};

struct OResize {
    template <typename T>
    static inline Expected<void> Exec(T& col, i64 len) {
        col.GetData().resize(len);
        return EError::NoError;
    }
};

struct OClear {
    template <typename TCol>
    static inline Expected<void> Exec(TCol& col) {
        col.GetData().clear();
        return EError::NoError;
    }
};

} // namespace JfEngine
