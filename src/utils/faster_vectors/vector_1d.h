#pragma once

#include "gstring.h"

#include <utils/compress/bitpack.h>
#include <utils/compress/dict.h>
#include <utils/cint/int.h>

#include <bit>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

class StringVector {
public:
    StringVector() = default;

    void push_back(std::string_view val) {
        offsets_.push_back(data_.size());
        data_.resize(data_.size() + val.size());
        std::memcpy(data_.data() + data_.size() - val.size(), val.data(), val.size());

    }

    void push_back_mcpy(void* addr, ui64 len) {
        offsets_.push_back(data_.size());
        data_.resize(data_.size() + len);
        std::memcpy(data_.data() + data_.size() - len, addr, len);
    }

    inline ui64 get_pos(ui64 i) const {
        return offsets_.at(i);
    }

    inline ui64 get_len(ui64 i) const {
        if (i + 1 == offsets_.size()) {
            return data_.size() - offsets_[i];
        } else {
            return offsets_[i + 1] - offsets_[i];
        }
    }

    std::string_view at(ui64 i) const {
        return std::string_view(data_.data() + offsets_[i], get_len(i));
    }

    std::string_view ro_at(ui64 i) const {
        return std::string_view(data_.data() + offsets_[i], get_len(i));
    }

    std::string operator[](ui64 i) const {
        return std::string(data_.data() + offsets_[i], get_len(i));
    }

    inline ui64 size() const {
        return offsets_.size();
    }

    inline void reserve(ui64 n) {
        offsets_.reserve(n);
    }

    inline void resize(ui64 n) {
        if (n == offsets_.size()) {
            return;
        }
        assert(n < offsets_.size());
        data_.resize(offsets_[n]);
        offsets_.resize(n);
    }

    inline void clear() {
        data_.clear();
        offsets_.clear();
    }

    inline char* data() {
        return data_.data();
    }

    inline const char* data() const {
        return data_.data();
    }

private:
    std::vector<char> data_;
    std::vector<ui64> offsets_;
};
