#pragma once

#include "utils/cint/int.h"
#include "utils/faster_vectors/gstring.h"

#include <string_view>
#include <vector>

struct OKmpSubstrFinder {
    static bool Exec(JString& haystack, std::string_view needle) {
        const ui32 N  = haystack.size();
        const ui32 nl = needle.size();
        if (nl == 0) return true;
        if (nl > N)  return false;

        const char* bytes = haystack.is_small()
            ? reinterpret_cast<const char*>(&haystack.prefix)
            : haystack.extra;

        std::vector<ui32> pf(nl, 0);
        for (ui32 i = 1; i < nl; i++) {
            ui32 j = pf[i - 1];
            while (j > 0 && needle[i] != needle[j]) j = pf[j - 1];
            if (needle[i] == needle[j]) j++;
            pf[i] = j;
        }

        ui32 j = 0;
        for (ui32 i = 0; i < N; i++) {
            char c = bytes[i];
            while (j > 0 && c != needle[j]) j = pf[j - 1];
            if (c == needle[j]) j++;
            if (j == nl) return true;
        }
        return false;
    }
};
