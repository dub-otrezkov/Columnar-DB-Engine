#include "types.h"

namespace JfEngine {

EColumn StrToTColumn(std::string_view data) {
    if (data == ki8SchemeAlias) {
        return ki8Column;
    }
    if (data == ki16SchemeAlias) {
        return ki16Column;
    }
    if (data == ki32SchemeAlias) {
        return ki32Column;
    }
    if (data == ki64SchemeAlias) {
        return ki64Column;
    }
    if (data == ki128SchemeAlias) {
        return ki128Column;
    }
    if (data == kDoubleSchemeAlias) {
        return kDoubleColumn;
    }
    if (data == kDateSchemeAlias) {
        return kDateColumn;
    }
    if (data == kTimestampSchemeAlias) {
        return kTimestampColumn;
    }
    if (data == kStringSchemeAlias) {
        return kStringColumn;
    }

    return kUnitialized;
}

std::string TColumnToStr(EColumn data) {
    switch (data) {
        case ki8Column:
            return ki8SchemeAlias;
        case ki16Column:
            return ki16SchemeAlias;
        case ki32Column:
            return ki32SchemeAlias;
        case ki64Column:
            return ki64SchemeAlias;
        case ki128Column:
            return ki128SchemeAlias;
        case kDoubleColumn:
            return kDoubleSchemeAlias;
        case kDateColumn:
            return kDateSchemeAlias;
        case kTimestampColumn:
            return kTimestampSchemeAlias;
        case kStringColumn:
            return kStringSchemeAlias;
        default:
            return kUnknownSchemeAlias;
    }
}

Expected<TColumnPtr> MakeEmptyColumn(EColumn type) {
    switch (type) {
        case ki8Column: {
            return std::make_shared<Ti8Column>();
        }
        case ki16Column: {
            return std::make_shared<Ti16Column>();
        }
        case ki32Column: {
            return std::make_shared<Ti32Column>();
        }
        case ki64Column: {
            return std::make_shared<Ti64Column>();
        }
        case ki128Column: {
            return std::make_shared<Ti128Column>();
        }
        case kDoubleColumn: {
            return std::make_shared<TDoubleColumn>();
        }
        case kDateColumn: {
            return std::make_shared<TDateColumn>();
        }
        case kTimestampColumn: {
            return std::make_shared<TTimestampColumn>();
        }
        case kStringColumn: {
            return std::make_shared<TStringColumn>();
        }
        default: {
            return MakeError<EError::UnsupportedErr>();
        }
    }
}

Expected<TColumnPtr> MakeColumn(std::vector<std::string> data, EColumn type) {
    switch (type) {
        case ki8Column: {
            return SetupColumn<Ti8Column>(std::move(data));
        }
        case ki16Column: {
            return SetupColumn<Ti16Column>(std::move(data));
        }
        case ki32Column: {
            return SetupColumn<Ti32Column>(std::move(data));
        }
        case ki64Column: {
            return SetupColumn<Ti64Column>(std::move(data));
        }
        case ki128Column: {
            return SetupColumn<Ti128Column>(std::move(data));
        }
        case kDoubleColumn: {
            return SetupColumn<TDoubleColumn>(std::move(data));
        }
        case kDateColumn: {
            return SetupColumn<TDateColumn>(std::move(data));
        }
        case kTimestampColumn: {
            return SetupColumn<TTimestampColumn>(std::move(data));
        }
        case kStringColumn: {
            return SetupColumn<TStringColumn>(std::move(data));
        }
        default: {
            return MakeError<EError::UnsupportedErr>();
        }
    }
}

Expected<TColumnPtr> MakeColumnOptimized(const TVectorString2d& data, ui64 column_i, EColumn type) {
    switch (type) {
        case ki8Column: {
            return SetupColumn<Ti8Column>(data, column_i);
        }
        case ki16Column: {
            return SetupColumn<Ti16Column>(data, column_i);
        }
        case ki32Column: {
            return SetupColumn<Ti32Column>(data, column_i);
        }
        case ki64Column: {
            return SetupColumn<Ti64Column>(data, column_i);
        }
        case ki128Column: {
            return SetupColumn<Ti128Column>(data, column_i);
        }
        case kDoubleColumn: {
            return SetupColumn<TDoubleColumn>(data, column_i);
        }
        case kDateColumn: {
            return SetupColumn<TDateColumn>(data, column_i);
        }
        case kTimestampColumn: {
            return SetupColumn<TTimestampColumn>(data, column_i);
        }
        case kStringColumn: {
            return SetupColumn<TStringColumn>(data, column_i);
        }
        default: {
            return MakeError<EError::UnsupportedErr>();
        }
    }
}

std::string PrintDate(const TDate& d) {
    std::string ans;

    for (ui8 i = 0; i < 4 - std::to_string(d.year).size(); i++) {
        ans += "0";
    }
    ans += std::to_string(d.year);
    ans += "-";
    if (d.month < 10) {
        ans += "0";
    }
    ans += std::to_string(d.month);
    ans += "-";
    if (d.day < 10) {
        ans += "0";
    }
    ans += std::to_string(d.day);

    return ans;
}

TDate DateFromStr(const std::string& s) {
    return TDate{
        static_cast<i16>(std::stoi(s.substr(0, 4))),
        static_cast<i8>(std::stoi(s.substr(5, 2))),
        static_cast<i8>(std::stoi(s.substr(8, 2)))
    };
}

std::string PrintTimestamp(const TTimestamp& d) {
    std::string ans = PrintDate(d.date);

    ans += " ";
    if (d.hour < 10) {
        ans += "0";
    }
    ans += std::to_string(d.hour);
    ans += ":";
    if (d.minute < 10) {
        ans += "0";
    }
    ans += std::to_string(d.minute);
    ans += ":";
    if (d.second < 10) {
        ans += "0";
    }
    ans += std::to_string(d.second);

    return ans;
}


TTimestamp TimestampFromStr(const std::string& s) {
    return TTimestamp{
        TDate{
            static_cast<i16>(std::stoi(s.substr(0, 4))),
            static_cast<i8>(std::stoi(s.substr(5, 2))),
            static_cast<i8>(std::stoi(s.substr(8, 2)))
        },
        static_cast<i8>(std::stoi(s.substr(11, 2))),
        static_cast<i8>(std::stoi(s.substr(14, 2))),
        static_cast<i8>(std::stoi(s.substr(17, 2)))
    };
}

Expected<TColumnPtr> MakeColumnJf(std::vector<char> data, EColumn type) {
    switch (type) {
        case kUnitialized: {
            return MakeError<EError::UnimplementedErr>();
        }
        case ki8Column: {
            return std::make_shared<Ti8Column>(Unserialize<i8>(data));
        }
        case ki16Column: {
            return std::make_shared<Ti16Column>(Unserialize<i16>(data));
        }
        case ki32Column: {
            return std::make_shared<Ti32Column>(Unserialize<i32>(data));
        }
        case ki64Column: {
            return std::make_shared<Ti64Column>(Unserialize<i64>(data));
        }
        case kDoubleColumn: {
            return std::make_shared<TDoubleColumn>(Unserialize<ld>(data));
        }
        case kDateColumn: {
            return std::make_shared<TDateColumn>(Unserialize<TDate>(data));
        }
        case kTimestampColumn: {
            return std::make_shared<TTimestampColumn>(Unserialize<TTimestamp>(data));
        }
        case kStringColumn: {
            return std::make_shared<TStringColumn>(UnserializeString(data));
        }
        default: {
            throw "wtf???";
        }
    }
}

} // namespace JfEngine
