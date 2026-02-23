#include "types.h"

namespace JFEngine {

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

Expected<IColumn> MakeColumn(std::vector<std::string> data, EColumn type) {
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
            return MakeError<UnsupportedErr>();
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

Expected<IColumn> MakeColumnJF(std::vector<std::string> data, EColumn type) {
    switch (type) {
        case kUnitialized: {
            return MakeError<UnimplementedErr>();
        }
        case ki8Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI8(data[i]));
            }
            break;
        }
        case ki16Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI16(data[i]));
            }
            break;
        }
        case ki32Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI32(data[i]));
            }
            break;
        }
        case ki64Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI64(data[i]));
            }
            break;
        }
        case kDoubleColumn: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToDouble(data[i]));
            }
            break;
        }
        case kDateColumn: {
            auto res = std::make_shared<TDateColumn>();
            for (ui64 i = 0; i < data.size(); i++) {
                std::stringstream ss;
                ss << data[i];

                TDate date{
                    ReadI16(ss),
                    ReadI8(ss),
                    ReadI8(ss),
                };

                data[i] = PrintDate(date);
            }
            break;
        }
        case kTimestampColumn: {
            auto res = std::make_shared<TTimestampColumn>();
            for (ui64 i = 0; i < data.size(); i++) {
                std::stringstream ss;
                ss << data[i];

                TTimestamp date;

                std::string ans;

                
                TTimestamp time{
                    TDate{
                        ReadI16(ss),
                        ReadI8(ss),
                        ReadI8(ss),
                    },
                    ReadI8(ss),
                    ReadI8(ss),
                    ReadI8(ss),
                };

                data[i] = PrintTimestamp(time);
            }
            break;
        }
        default: {
        }
        
    }

    return MakeColumn(std::move(data), type);
}

} // namespace JFEngine
