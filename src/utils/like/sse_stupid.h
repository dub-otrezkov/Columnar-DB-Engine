#pragma once

#include "utils/cint/int.h"
#include "utils/faster_vectors/gstring.h"

#include <nmmintrin.h>

// needle size <= 12

struct O128SubstrFinder {
    static bool Exec(JString& haystack, std::string_view needle) {
        if (needle.size() > haystack.size()) {
            return false;
        }
        __m128i ndl = _mm_loadu_si128((const __m128i*)needle.data());
        if (haystack.is_small()) {
            __m128i str = _mm_loadu_si128((const __m128i*)&haystack.prefix);

            return _mm_cmpestrc(ndl, needle.size(), str, haystack.size(), _SIDD_CMP_EQUAL_ORDERED | _SIDD_UBYTE_OPS);
        } else {
            const char* p   = haystack.extra;
            const char* end = p + haystack.size();
            const int step  = 16 - (needle.size() - 1);

            while (p < end) {
                __m128i chunk = _mm_loadu_si128((const __m128i*)p);
                if (_mm_cmpestrc(ndl, needle.size(), chunk, std::min(16l, end - p),
                                _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ORDERED)) {
                    return true;
                }
                p += step;
            }
            return false;
        }
    }
};