#include "types.h"

namespace JFEngine {

TColumn StrToTColumn(const std::string& data) {
    if (data == ki8SchemeAlias) {
        return Ei8Column;
    }
    if (data == ki16SchemeAlias) {
        return Ei16Column;
    }
    if (data == ki32SchemeAlias) {
        return Ei32Column;
    }
    if (data == ki64SchemeAlias) {
        return Ei64Column;
    }
    if (data == kDoubleSchemeAlias) {
        return EDoubleColumn;
    }
    if (data == kDateSchemeAlias) {
        return EDateColumn;
    }
    if (data == kTimestampSchemeAlias) {
        return ETimestampColumn;
    }
    if (data == kStringSchemeAlias) {
        return EStringColumn;
    }

    return EUnitialized;
}

std::string TColumnToStr(TColumn data) {
    switch (data) {
        case Ei8Column:
            return ki8SchemeAlias;
        case Ei16Column:
            return ki16SchemeAlias;
        case Ei32Column:
            return ki32SchemeAlias;
        case Ei64Column:
            return ki64SchemeAlias;
        case EDoubleColumn:
            return kDoubleSchemeAlias;
        case EDateColumn:
            return kDateSchemeAlias;
        case ETimestampColumn:
            return kTimestampSchemeAlias;
        case EStringColumn:
            return kStringSchemeAlias;
        default:
            return kUnknownSchemeAlias;
    }
}

Expected<IColumn> MakeColumn(std::vector<std::string> data, TColumn type) {
    switch (type) {
        case Ei8Column: {
            return SetupColumn<Ti8Column>(std::move(data));
        }
        case Ei16Column: {
            return SetupColumn<Ti16Column>(std::move(data));
        }
        case Ei32Column: {
            return SetupColumn<Ti32Column>(std::move(data));
        }
        case Ei64Column: {
            return SetupColumn<Ti64Column>(std::move(data));
        }
        case EDoubleColumn: {
            return SetupColumn<TDoubleColumn>(std::move(data));
        }
        case EDateColumn: {
            return SetupColumn<TDateColumn>(std::move(data));
        }
        case ETimestampColumn: {
            return SetupColumn<TTimestampColumn>(std::move(data));
        }
        case EStringColumn: {
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

Expected<IColumn> MakeColumnJF(std::vector<std::string> data, TColumn type) {
    switch (type) {
        case EUnitialized: {
            return MakeError<UnimplementedErr>();
        }
        case Ei8Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI8(data[i]));
            }
            break;
        }
        case Ei16Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI16(data[i]));
            }
            break;
        }
        case Ei32Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI32(data[i]));
            }
            break;
        }
        case Ei64Column: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToI64(data[i]));
            }
            break;
        }
        case EDoubleColumn: {
            auto res = std::make_shared<Ti64Column>();
            for (ui64 i = 0; i < data.size(); i++) {
                data[i] = std::to_string(JFStrToDouble(data[i]));
            }
            break;
        }
        case EDateColumn: {
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
        case ETimestampColumn: {
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
