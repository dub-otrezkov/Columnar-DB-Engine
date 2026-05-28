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
    auto ans = BitPack(data.size(), data, bits);
    ans.resize(ans.size() + sizeof(mn));
    ans.back() = sizeof(mn);
    std::memcpy(ans.data() + ans.size() - sizeof(mn), &mn);
    return ans;
}

template <std::integral T>
inline std::vector<T> DeltaUnserialize(const std::vector<char>& data) {
    T mn;
    std::memcpy(&mn, data.data() + data.size() - sizeof(T), sizeof(T));
    std::vector<T> ans;
    auto res = BitUnpack(data.size() - sizeof(T), data.data());
}
