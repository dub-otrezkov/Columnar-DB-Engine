#include "../types/types.h"


namespace JFEngine {

class OSum {
public:
    static Expected<IColumn> Exec(Ti8Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(Ti16Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(Ti32Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(Ti64Column& col) {
        i64 res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<Ti64Column>(std::vector<i64>{res});
    }

    static Expected<IColumn> Exec(TDoubleColumn& col) {
        ld res = 0;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<TDoubleColumn>(std::vector<ld>{res});
    }

    static Expected<IColumn> Exec(TDateColumn& col) {
        return MakeError<UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TTimestampColumn& col) {
        return MakeError<UnsupportedErr>();
    }

    static Expected<IColumn> Exec(TStringColumn& col) {
        std::string res;
        for (ui64 i = 0; i < col.GetSize(); i++) {
            res += col.GetData()[i];
        }
        return std::make_shared<TStringColumn>(std::vector<std::string>{res});
    }
};

} // namespace JFEngine