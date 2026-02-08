#include "int.h"

#include <sstream>

void PutI64(std::ostream& out, i64 i) {
    for (ui64 b = 0; b < sizeof(i64) / sizeof(char); b++) {
        out << char((i >> (b * 8)) & ((1 << 8) - 1));
    }
}

std::string I64ToJFStr(i64 i) {
    std::stringstream ss;
    PutI64(ss, i);
    auto res = ss.str();

    ss.clear();
    return res;
}


i64 JFStrToI64(const std::string& i) {
    std::stringstream ss;
    ss << i;
    return ReadI64(ss);
}


i64 ReadI64(std::istream& in) {
    i64 ans = 0;
    for (ui64 i = 0; i < sizeof(i64) / sizeof(char); i++) {
        i64 c = in.get();
        ans = (ans | (c << i * 8));
    }
    return ans;
}