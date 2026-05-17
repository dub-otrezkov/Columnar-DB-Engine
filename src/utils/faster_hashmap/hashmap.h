#pragma once

#include "utils/cint/int.h"

#include <cstdint>
#include <cstring>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename K, typename V, typename Hash = std::hash<K>>
class BetterHashMap {
    static constexpr ui8 EMPTY    = 0;
    static constexpr ui8 OCCUPIED = 1;

    // Группа из 16 байт состояний — для SIMD-like проверки
    // Используем верхние 7 бит хэша как "fingerprint" (h2)
    // Это позволяет отсеять ~99% ложных попаданий без доступа к ключу
    static constexpr ui8 H2_EMPTY = 0;

    static ui8 h2_from_hash(ui64 h) {
        // Берём верхние 7 бит, ставим бит 7 = 1 чтобы отличить от EMPTY
        return (ui8)((h >> 57) | 0x80u);
    }

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
            const auto* s = map->ctrl_;
            const auto cap = map->capacity_;
            while (pos < cap && s[pos] == H2_EMPTY) ++pos;
        }

        reference operator*() const {
            return {map->keys_[pos], map->values_[pos]};
        }

        Iterator& operator++() { ++pos; skip_empty(); return *this; }
        bool operator==(const Iterator& o) const { return pos == o.pos; }
        bool operator!=(const Iterator& o) const { return pos != o.pos; }
    };

    Iterator begin() { Iterator it{this, 0}; it.skip_empty(); return it; }
    Iterator end()   { return {this, capacity_}; }

    explicit BetterHashMap(ui64 initial_capacity = 16) {
        ui64 cap = 16;
        while (cap < initial_capacity) cap <<= 1;
        allocate(cap);
    }

    ~BetterHashMap() {
        if (__builtin_expect(keys_ != nullptr, 1)) {
            destroy_all();
            ::operator delete(static_cast<void*>(mem_));
        }
    }

    BetterHashMap(const BetterHashMap&) = delete;
    BetterHashMap& operator=(const BetterHashMap&) = delete;

    BetterHashMap(BetterHashMap&& o) noexcept
        : mem_(o.mem_), keys_(o.keys_), values_(o.values_), ctrl_(o.ctrl_),
          size_(o.size_), capacity_(o.capacity_), mask_(o.mask_), shift_(o.shift_) {
        o.mem_ = o.keys_ = nullptr;
        o.values_ = nullptr;
        o.ctrl_ = nullptr;
        o.size_ = o.capacity_ = o.mask_ = 0;
    }

    BetterHashMap& operator=(BetterHashMap&& o) noexcept {
        if (this != &o) {
            if (keys_) { destroy_all(); ::operator delete(mem_); }
            mem_ = o.mem_; keys_ = o.keys_; values_ = o.values_; ctrl_ = o.ctrl_;
            size_ = o.size_; capacity_ = o.capacity_; mask_ = o.mask_; shift_ = o.shift_;
            o.mem_ = nullptr; o.keys_ = nullptr; o.values_ = nullptr; o.ctrl_ = nullptr;
            o.size_ = o.capacity_ = o.mask_ = 0;
        }
        return *this;
    }

    template <typename KK, typename... Args>
    V& emplace(KK&& key, Args&&... args) {
        if (__builtin_expect(size_ * 10 > capacity_ * 7, 0)) {
            rehash(capacity_ << 1);
        }

        ui64 h = hasher_(key);
        ui64 idx = h1(h);
        ui8  tag = h2_from_hash(h);

        while (true) {
            ui8 c = ctrl_[idx];
            if (c == tag) {
                if (__builtin_expect(keys_[idx] == key, 1))
                    return values_[idx];
            } else if (c == H2_EMPTY) {
                break;
            }
            idx = (idx + 1) & mask_;
        }

        ctrl_[idx] = tag;
        new (&keys_[idx]) K(std::forward<KK>(key));
        new (&values_[idx]) V(std::forward<Args>(args)...);
        ++size_;
        return values_[idx];
    }

    V& operator[](const K& key) { return emplace(key); }

    V& at(const K& key) {
        ui64 h = hasher_(key);
        ui64 idx = h1(h);
        ui8  tag = h2_from_hash(h);

        while (true) {
            ui8 c = ctrl_[idx];
            if (c == tag && keys_[idx] == key) return values_[idx];
            if (c == H2_EMPTY) break;
            idx = (idx + 1) & mask_;
        }
        throw std::out_of_range("BetterHashMap::at");
    }

    const V& at(const K& key) const {
        ui64 h = hasher_(key);
        ui64 idx = h1(h);
        ui8  tag = h2_from_hash(h);

        while (true) {
            ui8 c = ctrl_[idx];
            if (c == tag && keys_[idx] == key) return values_[idx];
            if (c == H2_EMPTY) break;
            idx = (idx + 1) & mask_;
        }
        throw std::out_of_range("BetterHashMap::at");
    }

    bool contains(const K& key) const {
        if (__builtin_expect(capacity_ == 0, 0)) return false;
        ui64 h = hasher_(key);
        ui64 idx = h1(h);
        ui8  tag = h2_from_hash(h);

        while (true) {
            ui8 c = ctrl_[idx];
            if (c == tag && keys_[idx] == key) return true;
            if (c == H2_EMPTY) return false;
            idx = (idx + 1) & mask_;
        }
    }

    void reserve(ui64 n) {
        ui64 target = capacity_;
        while (target * 7 < n * 10) target <<= 1;
        if (target > capacity_) rehash(target);
    }

    void clear() {
        destroy_all();
        std::memset(ctrl_, H2_EMPTY, capacity_);
        size_ = 0;
    }

    ui64 size()  const { return size_; }
    bool empty() const { return size_ == 0; }

private:
    // h1: позиция в таблице (fibonacci hashing)
    ui64 h1(ui64 h) const {
        return (h * 0x9E3779B97F4A7C15ULL) >> shift_;
    }

    void allocate(ui64 cap) {
        capacity_ = cap;
        mask_ = cap - 1;
        shift_ = 64 - __builtin_ctzll(cap);

        // Один malloc: ctrl | padding | keys | values
        constexpr ui64 key_align = alignof(K) > 8 ? alignof(K) : 8;
        constexpr ui64 val_align = alignof(V) > 8 ? alignof(V) : 8;

        ui64 ctrl_bytes = cap;
        ui64 keys_off   = (ctrl_bytes + key_align - 1) & ~(key_align - 1);
        ui64 vals_off   = (keys_off + cap * sizeof(K) + val_align - 1) & ~(val_align - 1);
        ui64 total      = vals_off + cap * sizeof(V);

        mem_ = ::operator new(total);
        char* base = static_cast<char*>(mem_);
        ctrl_   = reinterpret_cast<ui8*>(base);
        keys_   = reinterpret_cast<K*>(base + keys_off);
        values_ = reinterpret_cast<V*>(base + vals_off);

        std::memset(ctrl_, H2_EMPTY, cap);
    }

    void destroy_all() {
        for (ui64 i = 0; i < capacity_; ++i) {
            if (ctrl_[i] != H2_EMPTY) {
                keys_[i].~K();
                values_[i].~V();
            }
        }
    }

    void rehash(ui64 new_cap) {
        void* old_mem = mem_;
        K*    old_keys = keys_;
        V*    old_values = values_;
        ui8*  old_ctrl = ctrl_;
        ui64  old_cap = capacity_;

        allocate(new_cap);
        size_ = 0;

        for (ui64 i = 0; i < old_cap; ++i) {
            if (old_ctrl[i] != H2_EMPTY) {
                emplace(std::move(old_keys[i]), std::move(old_values[i]));
                old_keys[i].~K();
                old_values[i].~V();
            }
        }

        ::operator delete(old_mem);
    }

    Hash  hasher_;
    void* mem_      = nullptr;
    K*    keys_     = nullptr;
    V*    values_   = nullptr;
    ui8*  ctrl_     = nullptr;
    ui64  size_     = 0;
    ui64  capacity_ = 0;
    ui64  mask_     = 0;
    ui64  shift_    = 64;
};