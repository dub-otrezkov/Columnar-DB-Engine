#include "int.h"

#include <sstream>

void PutI64(std::ostream& out, i64 i) {
    for (ui64 b = 0; b < sizeof(i64) / sizeof(char); b++) {
        out << char((i >> (b * sizeof(char))) & ((1 << sizeof(char)) - 1));
    }
}

std::string I64ToJFStr(i64 i) {
    std::stringstream ss;
    PutI64(ss, i);
    return ss.str();
}

i64 ReadI64(std::istream& in) {
    i64 ans = 0;
    for (ui64 i = 0; i < 8; i++) {
        ui64 c = in.get();
        ans = (ans | (c << i * 8));
    }
    return ans;
}