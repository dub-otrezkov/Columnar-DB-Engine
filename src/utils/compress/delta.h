#pragma once

#include "bitpack.h"

#include <utils/cint/int.h>
#include <utils/faster_vectors/gstring.h>

#include <bit>
#include <cstring>
#include <vector>

#include <boost/unordered/unordered_flat_map.hpp>

template <std::integral T>
inline std::vector<char> DeltaSerialize(ui32 n, T* data) {
    T mn = *std::min_element(data, data + n);
    T mx = *std::max_element(data, data + n);
    for (ui64 i = 0; i < n; i++) {
        data[i] -= mn;
    }
    auto t = BitPack(data, bits);
    return ans;
}

template <std::integral T>
inline std::vector<T> DeltaUnserialize(const std::vector<char>& data) {
    
}
