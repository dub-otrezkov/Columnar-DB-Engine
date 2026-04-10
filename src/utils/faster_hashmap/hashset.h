#pragma once

#include <utils/cint/int.h>

#include <vector>
#include <stdexcept>
#include <functional>

// LLM based code

template <typename K, typename Hash = std::hash<K>>
class BetterHashSet {
    struct Entry {
        K key;
        bool occupied = false;
    };

public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = const K;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const K*;
        using reference         = const K&;

        const BetterHashSet* map;
        ui64 pos;

        void skip_empty() {
            while (pos < map->capacity_ && !map->data_[pos].occupied) {
                ++pos;
            }
        }

        reference operator*()  const { return map->data_[pos].key; }
        pointer   operator->() const { return &map->data_[pos].key; }

        Iterator& operator++() {
            ++pos;
            skip_empty();
            return *this;
        }

        Iterator operator++(i32) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const { return pos == other.pos; }
        bool operator!=(const Iterator& other) const { return pos != other.pos; }
    };

    Iterator begin() const {
        Iterator it{this, 0};
        it.skip_empty();
        return it;
    }

    Iterator end() const {
        return {this, capacity_};
    }

    BetterHashSet() {
        rehash_to_size(get_next_prime(0));
    }

    bool insert(const K& key) {
        if (size_ * 2 > capacity_) {
            reserve(capacity_ * 2);
        }
        ui64 idx = find_index(key);
        if (data_[idx].occupied) return false;
        data_[idx].key      = key;
        data_[idx].occupied = true;
        ++size_;
        return true;
    }

    bool insert(K&& key) {
        if (size_ * 2 > capacity_) {
            reserve(capacity_ * 2);
        }
        ui64 idx = find_index(key);
        if (data_[idx].occupied) return false;
        data_[idx].key      = std::move(key);
        data_[idx].occupied = true;
        ++size_;
        return true;
    }

    bool contains(const K& key) const {
        if (capacity_ == 0) return false;
        ui64 idx = find_index(key);
        return data_[idx].occupied;
    }

    ui64 erase(const K& key) {
        if (capacity_ == 0) return 0;
        ui64 idx = find_index(key);
        if (!data_[idx].occupied) return 0;

        data_[idx].occupied = false;
        --size_;
        ui64 empty_idx = idx;
        ui64 cur = (idx + kStep) % capacity_;
        while (data_[cur].occupied) {
            ui64 natural = Hash{}(data_[cur].key) % capacity_;
            bool displaced = (empty_idx < cur)
                ? (natural <= empty_idx || natural > cur)
                : (natural <= empty_idx && natural > cur);
            if (displaced) {
                data_[empty_idx] = std::move(data_[cur]);
                data_[cur].occupied = false;
                empty_idx = cur;
            }
            cur = (cur + kStep) % capacity_;
        }
        return 1;
    }

    ui64  size()  const { return size_; }
    bool  empty() const { return size_ == 0; }

    void reserve(ui64 n) {
        ui64 new_capacity = get_next_prime(n);
        if (new_capacity > capacity_) {
            rehash_to_size(new_capacity);
        }
    }

    void clear() {
        for (auto& e : data_) e.occupied = false;
        size_ = 0;
    }

private:
    static ui64 get_next_prime(ui64 n) {
        static const ui64 primes[] = {
            53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157,
            98317, 196613, 393241, 786433, 1572869, 3145739, 6291469
        };
        for (ui64 p : primes) if (p > n) return p;
        return n;
    }

    ui64 find_index(const K& key) const {
        ui64 h   = Hash{}(key);
        ui64 idx = h % capacity_;
        ui64 cur = 1;
        while (data_[idx].occupied && !(data_[idx].key == key)) {
            idx = (idx + cur * cur) % capacity_;
            cur++;
        }
        return idx;
    }

    void rehash_to_size(ui64 new_capacity) {
        std::vector<Entry> old_data = std::move(data_);
        capacity_ = new_capacity;
        data_.assign(capacity_, {K(), false});
        size_ = 0;

        for (auto& entry : old_data) {
            if (entry.occupied) insert(std::move(entry.key));
        }
    }

    std::vector<Entry> data_;
    ui64 size_     = 0;
    ui64 capacity_ = 0;

    static constexpr ui64 kStep = 1;
};