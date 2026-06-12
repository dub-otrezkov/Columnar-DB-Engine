#pragma once

#include "string_heap.h"

#include <utils/cint/int.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string_view>
#include <vector>

#include <nmmintrin.h>
#include <functional>

#include <boost/unordered/unordered_flat_map.hpp>

struct JString {
    static constexpr ui64 kSmallStringSize = 12;

    explicit JString(ui32 size = 0, const char* data = nullptr) {
        len = size;
        if (size <= kSmallStringSize) {
            if (size > 0) {
                std::memcpy(&prefix, data, size);
            }
        } else {
            std::memcpy(&prefix, data, sizeof(prefix));
            auto extra_size = size;
            extra = TStringHeap::Allocate(extra_size);
            std::memcpy(extra, data, extra_size);
        }
    }

    explicit JString(std::string_view data) : JString(data.size(), data.data()) {
    }

    template<typename T>
    explicit JString(const T& data) : JString(sizeof(T), reinterpret_cast<const char*>(&data)) {
    }

    JString(JString&& other) :
        len(other.len),
        prefix(other.prefix),
        extra(other.extra)
    {
        other.extra = nullptr;
    }

    JString(const JString&) = default;

    JString& operator=(JString&& other) {
        len = other.len;
        prefix = other.prefix;
        extra = other.extra;

        other.extra = nullptr;

        return *this;
    }

    JString& operator=(const JString& other) = default;

    char* begin() {
        if (is_small()) {
            return reinterpret_cast<char*>(&prefix);
        } else {
            return extra;
        }
    }

    const char* begin() const {
        if (is_small()) {
            return reinterpret_cast<const char*>(&prefix);
        } else {
            return extra;
        }
    }

    char* end() {
        return begin() + len;
    }

    inline ui32 size() const {
        return len;
    }

    inline bool is_small() const {
        return len <= kSmallStringSize;
    }

    char& operator[] (ui64 i) {
        if (i >= len) {
            throw std::runtime_error("bad idx");
        }
        if (is_small()) {
            return *(reinterpret_cast<char*>(&prefix) + i);
        }
        return extra[i];
    }

    std::string to_string() const {
        if (len == 0) {
            return {};
        }
        if (is_small()) {
            return std::string(reinterpret_cast<const char*>(&prefix), len);
        }
        return std::string(extra, len);
    }

    ui32 len    = 0;
    ui32 prefix = 0;
    char* extra = 0;
};
static_assert(sizeof(JString) == 16);

inline const char* JStringBytes(const JString& s) {
    return s.is_small()
        ? reinterpret_cast<const char*>(&s.prefix)
        : s.extra;
}

inline bool operator== (const JString& i, const JString& j) {
    if (i.size() != j.size()) {
        return false;
    }
    if (i.prefix != j.prefix) {
        return false;
    }
    if (i.extra == j.extra) {
        return true;
    }
    if (i.size() <= sizeof(i.prefix)) {
        return true;
    }
    return std::memcmp(JStringBytes(i) + sizeof(i.prefix),
                       JStringBytes(j) + sizeof(j.prefix),
                       i.size() - sizeof(i.prefix)) == 0;
}

inline bool operator< (const JString& i, const JString& j) {
    ui32 min_len = std::min(i.size(), j.size());
    ui32 prefix_len = std::min(min_len, (ui32)sizeof(i.prefix));
    int c = memcmp(&i.prefix, &j.prefix, prefix_len);
    if (c != 0) {
        return c < 0;
    }
    if (min_len <= sizeof(i.prefix)) {
        return i.size() < j.size();
    }
    c = memcmp(JStringBytes(i) + sizeof(i.prefix),
               JStringBytes(j) + sizeof(j.prefix),
               min_len - sizeof(i.prefix));
    if (c != 0) {
        return c < 0;
    }
    return i.size() < j.size();
}

inline int JStringCompare(const JString& a, std::string_view b) {
    ui32 min_len = std::min<ui32>(a.size(), b.size());
    ui32 head = std::min<ui32>(min_len, sizeof(a.prefix));
    int c = std::memcmp(&a.prefix, b.data(), head);
    if (c != 0) {
        return c;
    }

    if (min_len > sizeof(a.prefix)) {
        c = std::memcmp(JStringBytes(a) + sizeof(a.prefix),
                        b.data() + sizeof(a.prefix),
                        min_len - sizeof(a.prefix));
        if (c != 0) {
            return c;
        }
    }
    if (a.size() < b.size()) {
        return -1;
    }
    if (a.size() > b.size()) {
        return  1;
    }
    return 0;
}

inline bool operator==(const JString& a, std::string_view b) {
    return a.size() == b.size() && JStringCompare(a, b) == 0;
}

inline bool operator==(std::string_view a, const JString& b) {
    return b == a;
}

inline bool operator!=(const JString& a, std::string_view b) {
    return !(a == b);
}
inline bool operator!=(std::string_view a, const JString& b) {
    return !(a == b);
}

inline bool operator< (const JString& a, std::string_view b) {
    return JStringCompare(a, b) <  0;
}
inline bool operator< (std::string_view a, const JString& b) {
    return JStringCompare(b, a) >  0;
}

inline bool operator<=(const JString& a, std::string_view b) {
    return JStringCompare(a, b) <= 0;
}
inline bool operator<=(std::string_view a, const JString& b) {
    return JStringCompare(b, a) >= 0;
}

inline bool operator> (const JString& a, std::string_view b) {
    return JStringCompare(a, b) >  0;
}
inline bool operator> (std::string_view a, const JString& b) {
    return JStringCompare(b, a) <  0;
}

inline bool operator>=(const JString& a, std::string_view b) {
    return JStringCompare(a, b) >= 0;
}
inline bool operator>=(std::string_view a, const JString& b) {
    return JStringCompare(b, a) <= 0;
}

inline bool operator<= (const JString& i, const JString& j) {
    return !(j < i);
}

struct JStringHasher {
    inline ui64 operator()(const JString& s) const noexcept {
        ui32 h = 0;
        h = _mm_crc32_u32(h, s.size());
        h = _mm_crc32_u32(h, s.prefix);

        if (s.size() <= sizeof(s.prefix)) {
            return h;
        }

        if (s.is_small()) {
            ui64 inline_tail;
            std::memcpy(&inline_tail, &s.extra, sizeof(inline_tail));
            return _mm_crc32_u64(h, inline_tail);
        }

        const char* tail = s.extra + sizeof(s.prefix);
        ui32 tail_size = s.size() - sizeof(s.prefix);
        ui64 h64 = h;
        ui32 i = 0;
        for (; i + 8 <= tail_size; i += 8) {
            ui64 v;
            std::memcpy(&v, tail + i, sizeof(v));
            h64 = _mm_crc32_u64(h64, v);
        }
        h = static_cast<ui32>(h64);
        for (; i < tail_size; i++) {
            h = _mm_crc32_u8(h, static_cast<unsigned char>(tail[i]));
        }
        return h;
    }
};

namespace std {
    template<>
    struct hash<JString> {
        inline size_t operator()(const JString& s) const noexcept {
            return JStringHasher{}(s);
        }
    };
}

inline std::size_t hash_value(const JString& s) noexcept {
    return JStringHasher{}(s);
}
