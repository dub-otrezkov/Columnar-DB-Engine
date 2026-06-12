#pragma once

#include "sse_stupid.h"
#include "kmp_stupid.h"

static constexpr ui64 k128Treshold = 12;

struct OLikeChecker {
    static bool Exec(JString& haystack, std::string_view needle) {
        if (needle.size() <= k128Treshold) {
            return O128SubstrFinder::Exec(haystack, needle);
        }
        return OKmpSubstrFinder::Exec(haystack, needle);
    };
    
};
