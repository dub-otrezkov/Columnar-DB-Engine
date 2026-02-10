#include "double.h"

void PutDouble(std::ostream& out, ld i) {
    static char mas[sizeof(ld)];
    memcpy(mas, &i, sizeof(ld));
    for (ui8 i = 0; i < sizeof(ld); i++) {
        out << mas[i];
    }
}

std::string DoubleToJFStr(ld i) {
    std::stringstream ss;
    PutDouble(ss, i);
    auto res = ss.str();
    return res;
}

ld ReadDouble(std::istream& in) {
    ld ans = 0;
    static char mas[sizeof(ld)];
    for (ui8 i = 0; i < sizeof(ld); i++) {
        mas[i] = in.get();
    }
    memcpy(&ans, mas, sizeof(ld));
    return ans;
}

ld JFStrToDouble(const std::string& i) {
    std::stringstream ss;
    ss << i;
    return ReadDouble(ss);
}

