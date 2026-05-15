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

struct JString {
    static constexpr ui64 kSmallStringSize = 12;

    ui32 len    = 0;
    ui32 prefix = 0;
    char* extra = 0;

    explicit JString(ui32 size = 0, const char* data = nullptr) {
        len = size;
        if (size <= kSmallStringSize) {
            std::memcpy(&prefix, data, size);
        } else {
            std::memcpy(&prefix, data, sizeof(prefix));
            auto extra_size = size;
            extra = TStringHeap::Allocate(extra_size);
            std::memcpy(extra, data, extra_size);
        }
    }

    explicit JString(std::string_view data) : JString(data.size(), data.data()) {
    }

    explicit JString(std::string data) : JString(data.size(), data.data()) {
    }

    explicit JString(const std::string& data) : JString(data.size(), data.data()) {
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
    if (c != 0) return c;

    if (min_len > sizeof(a.prefix)) {
        c = std::memcmp(JStringBytes(a) + sizeof(a.prefix),
                        b.data() + sizeof(a.prefix),
                        min_len - sizeof(a.prefix));
        if (c != 0) return c;
    }
    if (a.size() < b.size()) return -1;
    if (a.size() > b.size()) return  1;
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
            h64 = _mm_crc32_u64(h64, *reinterpret_cast<const ui64*>(tail + i));
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

// namespace boost {
//     template <>
//     struct hash<JString> {
//         inline std::size_t operator()(const JString& s) const noexcept {
//             return JStringHasher{}(s);
//         }
//     };
// }

class JStringVector {
public:
    JStringVector() = default;
    JStringVector(JStringVector&&) = default;
    JStringVector(const JStringVector&) = default;
    JStringVector& operator=(const JStringVector&) = default;

    JStringVector& operator=(JStringVector&& other) {
        data_ = std::move(other.data_);
        return *this;
    }

    inline void push_back(std::string_view data) {
        data_.emplace_back(data);
    }

    inline void push_back(JString&& data) {
        data_.push_back(std::move(data));
    }

    inline void push_back(const JString& data) {
        data_.push_back(data);
    }

    template<typename ...Args>
    inline void emplace_back(Args&&... data) {
        data_.emplace_back(std::forward<Args>(data)...);
    }

    inline const JString& at(ui64 i) const {
        return data_.at(i);
    }

    inline JString& at(ui64 i) {
        return data_.at(i);
    }

    inline ui64 size() const {
        return data_.size();
    }

    inline void reserve(ui64 n) {
        data_.reserve(n);
    }

    inline void resize(ui64 n) {
        data_.resize(n);
    }

    inline void clear() {
        data_.clear();
    }

    inline JString* data() {
        return data_.data();
    }

    inline const JString* data() const {
        return data_.data();
    }

    inline static std::vector<char> Serialize(const std::vector<JString>& data) {
        std::vector<char> ans;
        for (ui64 i = 0; i < data.size(); i++) {
            auto& cur = data.at(i);
            ui32 sz = cur.size();
            auto old_size = ans.size();
            ans.resize(old_size + sizeof(sz) + sz);
            std::memcpy(ans.data() + old_size, &sz, sizeof(sz));
            old_size += sizeof(sz);
            if (cur.is_small()) {
                std::memcpy(ans.data() + old_size, &cur.prefix, sz);
            } else {
                std::memcpy(ans.data() + old_size, cur.extra, sz);
            }
        }
        return ans;
    }

    inline static std::vector<JString> Unserialize(const std::vector<char>& data) {
        std::vector<JString> ans;
        ui64 i = 0;

        ui32 sz = 0;
        while (i < data.size()) {
            std::memcpy(&sz, data.data() + i, sizeof(sz));
            i += sizeof(sz);
            ans.emplace_back(sz, data.data() + i);
            i += sz;
        }

        return ans;
    }
private:
    std::vector<JString> data_;
};