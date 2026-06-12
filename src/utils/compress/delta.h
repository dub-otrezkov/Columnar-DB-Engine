#pragma once

#include "bitpack.h"

#include <utils/cint/int.h>

#include <algorithm>
#include <concepts>
#include <cstring>
#include <type_traits>
#include <vector>

template <std::integral T>
inline std::vector<char> DeltaSerialize(ui64 n, T* data) {
    using U = std::make_unsigned_t<T>;
    T mn = (n == 0) ? T{} : *std::min_element(data, data + n);
    for (ui64 i = 0; i < n; i++) {
        data[i] = static_cast<T>(static_cast<U>(data[i]) - static_cast<U>(mn));
    }
    auto packed = BitPack(n, data);

    std::vector<char> ans(sizeof(mn) + packed.size());
    std::memcpy(ans.data(), &mn, sizeof(mn));
    std::memcpy(ans.data() + sizeof(mn), packed.data(), packed.size());
    return ans;
}

template <std::integral T>
inline std::vector<T> DeltaUnserialize(size_t size, const char* data) {
    using U = std::make_unsigned_t<T>;
    if (size < sizeof(T)) {
        return {};
    }
    T mn;
    std::memcpy(&mn, data, sizeof(mn));

    std::vector<T> res;
    BitUnpack(size - sizeof(mn), data + sizeof(mn), res);
    for (auto& v : res) {
        v = static_cast<T>(static_cast<U>(v) + static_cast<U>(mn));
    }
    return res;
}
