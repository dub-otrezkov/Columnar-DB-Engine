#pragma once

#include <utils/cint/int.h>
#include <vector>
#include <string>
#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>

template <typename K, typename V, typename Hash = std::hash<K>>
class BetterHashMap {
    struct Entry {
        K key;
        V value;
    };

public:
    struct ValueProxy {
        const K& first;
        V& second;
    };

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueProxy;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = ValueProxy;

        BetterHashMap* map;
        ui64 pos;

        void skip_empty() {
            while (pos < map->capacity_ && !map->data_[pos].has_value()) {
                pos++;
            }
        }

        reference operator*() const {
            return {map->data_[pos]->key, map->data_[pos]->value};
        }

        Iterator& operator++() {
            pos++;
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

    Iterator begin() {
        Iterator it{this, 0};
        it.skip_empty();
        return it;
    }

    Iterator end() {
        return {this, capacity_};
    }

    BetterHashMap() {
        rehash_to_size(get_next_prime(0));
    }

    // emplace для поддержки синтаксиса: groups_.emplace(key, TGroup{...})
    template <typename... Args>
    V& emplace(const K& key, Args&&... args) {
        if (size_ * 2 > capacity_) {
            reserve(capacity_ * 2);
        }

        ui64 idx = find_index(key);
        if (!data_[idx].has_value()) {
            // Конструируем Entry на месте
            data_[idx].emplace(Entry{key, V(std::forward<Args>(args)...)});
            size_++;
        }
        return data_[idx]->value;
    }

    V& operator[](const K& key) {
        return emplace(key);
    }

    V& at(const K& key) {
        ui64 idx = find_index(key);
        if (!data_[idx].has_value()) throw std::out_of_range("BetterHashMap::at");
        return data_[idx]->value;
    }

    ui64 size() const { return size_; }
    bool empty() const { return size_ == 0; }

    bool contains(const K& key) const {
        if (capacity_ == 0) return false;
        ui64 idx = find_index(key);
        return data_[idx].has_value() && data_[idx]->key == key;
    }

    void reserve(ui64 n) {
        ui64 new_capacity = get_next_prime(n);
        if (new_capacity > capacity_) {
            rehash_to_size(new_capacity);
        }
    }

private:
    static ui64 get_next_prime(ui64 n) {
        static const std::vector<ui64> primes = {
            53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 
            98317, 196613, 393241, 786433, 1572869, 3145739, 6291469
        };
        for (ui64 p : primes) if (p > n) return p;
        return n > 0 ? n * 2 + 1 : 53; // Фоллбек
    }

    ui64 find_index(const K& key) const {
        ui64 h = Hash{}(key);
        ui64 idx = h % capacity_;
        ui64 cur = 1;
        while (data_[idx].has_value() && !(data_[idx]->key == key)) {
            idx = (idx + cur * cur) % capacity_;
            cur++;
        }
        return idx;
    }

    void rehash_to_size(ui64 new_capacity) {
        std::vector<std::optional<Entry>> old_data = std::move(data_);
        capacity_ = new_capacity;

        data_.assign(capacity_, std::nullopt);
        size_ = 0;

        for (auto& opt_entry : old_data) {
            if (opt_entry.has_value()) {
                emplace(std::move(opt_entry->key), std::move(opt_entry->value));
            }
        }
    }

    std::vector<std::optional<Entry>> data_;
    ui64 size_ = 0;
    ui64 capacity_ = 0;
};