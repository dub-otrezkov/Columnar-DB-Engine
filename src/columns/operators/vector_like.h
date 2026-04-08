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

// from, to
struct OPushBackVector {
    template<typename TCol>
    static inline void Exec(TCol& from, TColumnPtr to) {
        if (to->GetType() != from.GetType()) {
            throw "bad arg";
        }
        for (ui64 i = 0; i < from.GetSize(); i++) {
            OPushBack::Exec(*static_cast<TCol*>(to.get()), from.GetData()[i]);
        }
    }
};

struct OResize {
    template <typename T>
    static inline void Exec(T& col, i64 len) {
        col.GetData().resize(len);
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
