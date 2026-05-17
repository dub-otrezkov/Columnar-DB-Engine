#include "vector_1d.h"

std::string operator+(std::string_view a, std::string_view b) {
    std::string res(a.length() + b.length(), '0');
    std::memcpy(res.data(), a.data(), a.length());
    std::memcpy(res.data() + a.length(), b.data(), b.length());
    return res;
}
