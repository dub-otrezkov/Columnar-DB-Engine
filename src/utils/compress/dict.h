#pragma once

#include "bitpack.h"

#include <utils/cint/int.h>
#include <utils/faster_vectors/gstring.h>

#include <bit>
#include <cstring>
#include <vector>

#include <boost/unordered/unordered_flat_map.hpp>

inline std::vector<char> DictSerialize(ui64 n, JString* data) {
    constexpr ui64 kDictMaxSize = 1ULL << 13;
    ui64 cap = std::min<ui64>(n, kDictMaxSize);
    boost::unordered_flat_map<JString, ui16> idx;
    idx.reserve(cap);
    std::vector<JString> dict_ordered;
    dict_ordered.reserve(cap);
    std::vector<ui16> indices;
    indices.reserve(n);

    bool use_dict = true;
    for (ui64 i = 0; i < n; i++) {
        const auto& el = data[i];
        auto [it, inserted] = idx.try_emplace(el, 0);
        if (inserted) {
            if (idx.size() > kDictMaxSize) {
                use_dict = false;
                break;
            }
            it->second = static_cast<ui16>(dict_ordered.size());
            dict_ordered.push_back(el);
        }
        indices.push_back(it->second);
    }

    std::vector<char> ans;
    auto write_raw = [&](const void* src, ui64 sz) {
        auto off = ans.size();
        ans.resize(off + sz);
        if (sz > 0) {
            std::memcpy(ans.data() + off, src, sz);
        }
    };
    auto write_u32 = [&](ui32 v) { write_raw(&v, sizeof(v)); };

    if (!use_dict) {
        ans.push_back(0);
        write_u32(static_cast<ui32>(n));
        for (ui64 i = 0; i < n; i++) {
            const auto& el = data[i];
            write_u32(el.size());
            write_raw(el.begin(), el.size());
        }
    } else {
        ans.push_back(1);
        write_u32(static_cast<ui32>(dict_ordered.size()));
        for (const auto& el : dict_ordered) {
            write_u32(el.size());
            write_raw(el.begin(), el.size());
        }

        ui32 dict_count = static_cast<ui32>(dict_ordered.size());
        ui32 bits = (dict_count <= 1) ? 0 : static_cast<ui32>(std::bit_width(dict_count - 1));
        ui8 bits_byte = static_cast<ui8>(bits);
        write_raw(&bits_byte, sizeof(bits_byte));

        auto packed = BitPack(indices.size(), indices.data(), bits);
        write_raw(packed.data(), packed.size());
    }
    return ans;
}

inline std::vector<JString> DictUnserialize(size_t size, char* data) {
    std::vector<JString> ans;
    if (size == 0) {
        return ans;
    }

    ui64 i = 0;
    auto read_raw = [&](void* dst, ui64 sz) {
        std::memcpy(dst, data + i, sz);
        i += sz;
    };
    auto read_u32 = [&]() -> ui32 {
        ui32 v;
        read_raw(&v, sizeof(v));
        return v;
    };

    ui8 marker = static_cast<ui8>(data[i++]);

    if (marker == 0) {
        ui32 row_count = read_u32();
        ans.reserve(row_count);
        for (ui32 r = 0; r < row_count; r++) {
            ui32 sz = read_u32();
            ans.emplace_back(sz, data + i);
            i += sz;
        }
    } else {
        ui32 dict_count = read_u32();
        std::vector<JString> dict;
        dict.reserve(dict_count);
        for (ui32 k = 0; k < dict_count; k++) {
            ui32 sz = read_u32();
            dict.emplace_back(sz, data + i);
            i += sz;
        }

        ui8 bits = static_cast<ui8>(data[i++]);
        std::vector<ui16> indices;
        BitUnpack(size - i, data + i, bits, indices);

        ans.reserve(indices.size());
        for (auto ix : indices) {
            ans.push_back(dict.at(ix));
        }
    }
    return ans;
}
