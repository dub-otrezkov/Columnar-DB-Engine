#pragma once

#include <utils/cint/int.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string_view>
#include <vector>

struct JString {
    ui32 len_    = 0;
    ui32 offset_ = 0;
    ui64 prefix_ = 0;

    ui32 size() const {
        return len_;
    }
};
static_assert(sizeof(JString) == 16);

class JStringVector {
public:
    JStringVector() = default;
    JStringVector(JStringVector&&) = default;
    JStringVector(const JStringVector&) = default;
    JStringVector& operator=(const JStringVector&) = default;

    void push_back(std::string_view val) {
        const ui32 len    = static_cast<ui32>(val.size());
        const ui32 offset = static_cast<ui32>(extra_.size());

        JString s;
        s.len_    = len;
        s.offset_ = offset;
        std::memcpy(&s.prefix_, val.data(), std::min(len, 8u));

        data_.push_back(s);

        if (len > 8) {
            extra_.resize(offset + len - 8);
            std::memcpy(extra_.data() + offset, val.data() + 8, len - 8);
        }
    }

    void push_back(const JString& val) {
        const ui32 offset = static_cast<ui32>(extra_.size());

        JString s  = val;
        s.offset_  = offset;
        data_.push_back(s);

        if (val.len_ > 8) {
            extra_.resize(offset + val.len_ - 8);
            std::memcpy(
                extra_.data() + offset,
                extra_.data() + val.offset_,
                val.len_ - 8
            );
        }
    }

    bool equal(ui64 i, std::string_view other) const {
        const JString& s = data_[i];
        if (s.len_ != static_cast<ui32>(other.size())) { return false; }
        ui64 other_prefix = 0;
        std::memcpy(&other_prefix, other.data(), std::min(s.len_, 8u));
        if (s.prefix_ != other_prefix) { return false; }
        if (s.len_ <= 8) { return true; }
        return std::memcmp(
            extra_.data() + s.offset_,
            other.data() + 8,
            s.len_ - 8
        ) == 0;
    }

    bool equal(ui64 i, ui64 j) const {
        const JString& a = data_[i];
        const JString& b = data_[j];
        if (a.len_ != b.len_) { return false; }
        if (a.prefix_ != b.prefix_) { return false; }
        if (a.len_ <= 8) { return true; }
        return std::memcmp(
            extra_.data() + a.offset_,
            extra_.data() + b.offset_,
            a.len_ - 8
        ) == 0;
    }

    int compare(ui64 i, ui64 j) const {
        const JString& a = data_[i];
        const JString& b = data_[j];
        const ui32 common = std::min({a.len_, b.len_, 8u});
        const int prefix_cmp = std::memcmp(&a.prefix_, &b.prefix_, common);
        if (prefix_cmp != 0) { return prefix_cmp; }
        if (a.len_ > 8 || b.len_ > 8) {
            const ui32 tail_common = std::min(
                a.len_ > 8 ? a.len_ - 8 : 0u,
                b.len_ > 8 ? b.len_ - 8 : 0u
            );
            if (tail_common > 0) {
                const int tail_cmp = std::memcmp(
                    extra_.data() + a.offset_,
                    extra_.data() + b.offset_,
                    tail_common
                );
                if (tail_cmp != 0) { return tail_cmp; }
            }
        }
        if (a.len_ < b.len_) { return -1; }
        if (a.len_ > b.len_) { return  1; }
        return 0;
    }

    int compare(ui64 i, std::string_view other) const {
        const JString& s     = data_[i];
        const ui32 other_len = static_cast<ui32>(other.size());
        ui64 other_prefix    = 0;
        std::memcpy(&other_prefix, other.data(), std::min(other_len, 8u));
        const ui32 common    = std::min({s.len_, other_len, 8u});
        const int prefix_cmp = std::memcmp(&s.prefix_, &other_prefix, common);
        if (prefix_cmp != 0) { return prefix_cmp; }
        if (s.len_ > 8 || other_len > 8) {
            const ui32 tail_common = std::min(
                s.len_    > 8 ? s.len_    - 8 : 0u,
                other_len > 8 ? other_len - 8 : 0u
            );
            if (tail_common > 0) {
                const int tail_cmp = std::memcmp(
                    extra_.data() + s.offset_,
                    other.data()  + 8,
                    tail_common
                );
                if (tail_cmp != 0) { return tail_cmp; }
            }
        }
        if (s.len_ < other_len) { return -1; }
        if (s.len_ > other_len) { return  1; }
        return 0;
    }

    std::string get(ui64 i) const {
        const JString& s = data_[i];
        std::string result(s.len_, '\0');
        std::memcpy(result.data(), &s.prefix_, std::min(s.len_, 8u));
        if (s.len_ > 8) {
            std::memcpy(result.data() + 8, extra_.data() + s.offset_, s.len_ - 8);
        }
        return result;
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

    inline ui64 get_len(ui64 i) const {
        return data_.at(i).len_;
    }

    inline void reserve(ui64 n) {
        data_.reserve(n);
    }

    inline void reserve_extra(ui64 bytes) {
        extra_.reserve(bytes);
    }

    inline void resize(ui64 n) {
        if (n == data_.size()) { return; }
        if (n < data_.size()) {
            extra_.resize(n == 0 ? 0u : data_[n].offset_);
            data_.resize(n);
        } else {
            const ui32 offset = static_cast<ui32>(extra_.size());
            data_.resize(n, JString{0, 0, offset});
        }
    }

    inline void clear() {
        data_.clear();
        extra_.clear();
    }

    static std::vector<char> Serialize(const JStringVector& data) {
        std::vector<char> ans(data.data_.size() * sizeof(JString) + data.extra_.size() + sizeof(i64));
        std::memcpy(ans.data(), data.extra_.data(), data.extra_.size());
        std::memcpy(
            ans.data() + data.extra_.size(),
            reinterpret_cast<const char*>(data.data_.data()),
            data.data_.size() * sizeof(JString)
        );
        ui64 len = data.size();
        std::memcpy(
            ans.data() + data.data_.size() * sizeof(JString) + data.extra_.size(),
            reinterpret_cast<const char*>(&len),
            sizeof(len)
        );
    }

    static JStringVector Unserialize(const std::vector<char>& data) {
        JStringVector ans;
        ui64 len = *reinterpret_cast<const ui64*>(data.data() + data.size() - sizeof(len));
        ans.data_.resize(len);
        ans.extra_.resize(data.size() - len * sizeof(JString) - sizeof(len));
        std::memcpy(
            ans.extra_.data(),
            data.data(),
            ans.extra_.size()
        );
        std::memcpy(
            ans.data_.data(),
            data.data() + ans.extra_.size(),
            ans.data_.size() * sizeof(JString)
        );
    }
private:
    std::vector<JString> data_;
    std::vector<char>    extra_;
};