#pragma once

#include <utils/cint/int.h>

#include <vector>
#include <string>
#include <cassert>
#include <cstdint>

template <typename T>
struct FlatVectorImpl {
    using Type = std::vector<T>;
};

class StringVector {
public:
    StringVector() = default;

    void push_back(std::string_view val) {
        offsets_.push_back(data_.size());
        data_.insert(data_.end(), val.begin(), val.end());
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

    std::string at(ui64 i) const {
        return std::string(data_.data() + offsets_[i], get_len(i));
    }

    std::string operator[](ui64 i) const {
        return at(i);
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

    struct Iterator {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = std::string_view;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = std::string_view;

        const StringVector* col;
        ui64 pos;

        reference operator*() const {
            return col->at(pos);
        }

        Iterator& operator++() {
            pos++;
            return *this;
        }

        Iterator operator++(i32) {
            Iterator tmp = *this;
            pos++;
            return tmp;
        }

        Iterator& operator--() {
            pos--;
            return *this;
        }

        Iterator operator--(i32) {
            Iterator tmp = *this;
            pos--;
            return tmp;
        }

        Iterator& operator+=(difference_type n) {
            pos += n;
            return *this;
        }

        Iterator& operator-=(difference_type n) {
            pos -= n;
            return *this;
        }

        Iterator operator+(difference_type n) const {
            return {col, pos + n};
        }

        Iterator operator-(difference_type n) const {
            return {col, pos - n};
        }

        difference_type operator-(const Iterator& other) const {
            return (difference_type)pos - (difference_type)other.pos;
        }
        
        bool operator==(const Iterator& other) const {
            return pos == other.pos;
        }

        bool operator!=(const Iterator& other) const {
            return pos != other.pos;
        }

        bool operator<(const Iterator& other) const {
            return pos < other.pos;
        }

        bool operator>(const Iterator& other) const {
            return pos > other.pos;
        }

        bool operator<=(const Iterator& other) const {
            return pos <= other.pos;
        }

        bool operator>=(const Iterator& other) const {
            return pos >= other.pos;
        }
    };

    Iterator begin() const {
        return {this, 0};
    }

    Iterator end() const {
        return {this, size()};
    }

    friend std::vector<char> Serialize(const StringVector& a);
    friend StringVector UnserializeString(const std::vector<char>& a);

    inline char* data() {
        return data_.data();
    }

    inline ui64* offsets_data() {
        return offsets_.data();
    }

    inline void resize_both(ui64 data_sz, ui64 offsets_sz) {
        data_.resize(data_sz);
        offsets_.resize(offsets_sz);
    }

    inline ui64 data_size() const {
        return data_.size();
    }

    bool operator==(const StringVector& other) const {
        if (other.size() != size()) {
            return false;
        }
        if (other.data_size() != data_size()) {
            return false;
        }

        return (data_ == other.data_ && offsets_ == other.offsets_);
    }
private:
    std::vector<char> data_;
    std::vector<ui64> offsets_;
};

template <>
struct FlatVectorImpl<std::string> {
    using Type = StringVector;
};

std::string operator+(std::string_view a, std::string_view b);

template <typename T>
using FlatVector = typename FlatVectorImpl<T>::Type;

template <typename T>
std::vector<char> Serialize(const std::vector<T>& a) {
    std::vector<char> res(a.size() * sizeof(T));
    std::memcpy(res.data(), reinterpret_cast<const char*>(a.data()), res.size());
    return std::move(res);
}

template <typename T>
std::vector<T> Unserialize(const std::vector<char>& a) {
    std::vector<T> res(a.size() / sizeof(T));
    std::memcpy(res.data(), a.data(), a.size());
    return std::move(res);
}

std::vector<char> Serialize(const StringVector& a);
StringVector UnserializeString(const std::vector<char>& a);
