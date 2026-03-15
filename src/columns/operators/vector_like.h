#include "operators.h"

#include "../types/types.h"

namespace JFEngine {

struct OPushBack {
    static void Exec(Ti8Column& col, i8 value) {
        col.GetData().push_back(value);
    }

    static void Exec(Ti16Column& col, i16 value) {
        col.GetData().push_back(value);
    }

    static void Exec(Ti32Column& col, i32 value) {
        col.GetData().push_back(value);
    }

    static void Exec(Ti64Column& col, i64 value) {
        col.GetData().push_back(value);
    }

    static void Exec(TDoubleColumn& col, ld value) {
        col.GetData().push_back(value);
    }

    static void Exec(TDateColumn& col, TDate value) {
        col.GetData().push_back(value);
    }

    static void Exec(TTimestampColumn& col, TTimestamp value) {
        col.GetData().push_back(value);
    }

    static void Exec(TStringColumn& col, std::string value) {
        col.GetData().push_back(value);
    }

    template<typename T1, typename T2>
    static void Exec(T1& col, T2 value) {
        throw "bad arg";
    }
};

struct OFront {
    static std::shared_ptr<IColumn> Exec(Ti8Column& col) {
        return std::make_shared<Ti8Column>(std::vector<i8>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(Ti16Column& col) {
        return std::make_shared<Ti16Column>(std::vector<i16>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(Ti32Column& col) {
        return std::make_shared<Ti32Column>(std::vector<i32>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(Ti64Column& col) {
        return std::make_shared<Ti64Column>(std::vector<i64>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(TDoubleColumn& col) {
        return std::make_shared<TDoubleColumn>(std::vector<ld>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(TDateColumn& col) {
        return std::make_shared<TDateColumn>(std::vector<TDate>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(TTimestampColumn& col) {
        return std::make_shared<TTimestampColumn>(std::vector<TTimestamp>{col.GetData()[0]});
    }

    static std::shared_ptr<IColumn> Exec(TStringColumn& col) {
        return std::make_shared<TStringColumn>(std::vector<std::string>{col.GetData()[0]});
    }
};

// from, to, i
struct OPushBackFrom {
    static void Exec(Ti8Column& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(Ti16Column& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(Ti32Column& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(Ti64Column& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(TDoubleColumn& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(TDateColumn& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(TTimestampColumn& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }

    static void Exec(TStringColumn& from, TColumnPtr to, i64 i) {
        Do<OPushBack>(to, from.GetData()[i]);
    }
};

struct OClear {
    static Expected<void> Exec(Ti8Column& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(Ti16Column& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(Ti32Column& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(Ti64Column& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(TDoubleColumn& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(TDateColumn& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(TTimestampColumn& col) {
        col.GetData().clear();
        return EError::NoError;
    }

    static Expected<void> Exec(TStringColumn& col) {
        col.GetData().clear();
        return EError::NoError;
    }
};

} // namespace JFEngine
