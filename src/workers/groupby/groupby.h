#pragma once

#include "columns/operators/filter.h"
#include "columns/operators/operators.h"
#include "columns/types/types.h"
#include "csvio/csv_reader.h"
#include "utils/faster_hashmap/hashmap.h"
#include "utils/faster_vectors/gstring.h"
#include "workers/ao_engine/engine.h"
#include "workers/base.h"

#include <boost/unordered/unordered_flat_map.hpp>
#include <nmmintrin.h>

namespace JfEngine {

constexpr ui64 kUnlimited = -1;

struct TGroupByQuery {
    std::vector<std::string> cols;
    ui64 limit = kUnlimited;
    bool is_id = false;
};

// Per-row hash of a typed value, byte-equivalent to JStringHasher{}(JString(sizeof(T), &val)).
template<typename T>
inline ui64 HashTypedVal(const T& val) {
    constexpr ui32 sz = sizeof(T);
    ui32 h = _mm_crc32_u32(0, sz);
    if constexpr (sz <= 4) {
        ui32 prefix = 0;
        std::memcpy(&prefix, &val, sz);
        return _mm_crc32_u32(h, prefix);
    } else if constexpr (sz <= 12) {
        ui32 prefix = 0;
        std::memcpy(&prefix, &val, 4);
        h = _mm_crc32_u32(h, prefix);
        ui64 inline_tail = 0;
        std::memcpy(&inline_tail, reinterpret_cast<const char*>(&val) + 4, sz - 4);
        return _mm_crc32_u64(h, inline_tail);
    } else {
        ui32 prefix = 0;
        std::memcpy(&prefix, &val, 4);
        h = _mm_crc32_u32(h, prefix);
        const char* tail = reinterpret_cast<const char*>(&val) + 4;
        constexpr ui32 tail_size = sz - 4;
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
}

// Combine per-col hash into per-row accumulator in-place via XOR-shift fold.
// Matches the combine used by VectorStringHashed so RowView and StoredKey produce identical hashes.
struct OHashInto {
    template<typename TCol>
    static inline Expected<void> Exec(TCol& col, std::vector<ui64>& hashes) {
        auto& d = col.GetData();
        for (ui64 i = 0; i < d.size(); i++) {
            ui64 h = HashTypedVal(d[i]);
            hashes[i] = hashes[i] ^ h ^ (hashes[i] << 13);
        }
        return EError::NoError;
    }

    static inline Expected<void> Exec(TStringColumn& col, std::vector<ui64>& hashes) {
        auto& d = col.GetData();
        JStringHasher hsh;
        for (ui64 i = 0; i < d.size(); i++) {
            ui64 h = hsh(d[i]);
            hashes[i] = hashes[i] ^ h ^ (hashes[i] << 13);
        }
        return EError::NoError;
    }
};

struct TStoredKey {
    std::vector<JString> vals;
    ui64 hash;
};

struct TRowView {
    const std::vector<TColumnPtr>* cols;
    const std::vector<i64>* col_idxs;
    ui64 row;
    ui64 hash;

    inline std::pair<const char*, ui32> BytesAt(ui64 c) const {
        IColumn* col = (*cols)[(*col_idxs)[c]].get();
        switch (col->GetType()) {
            case ki8Column:
                return {reinterpret_cast<const char*>(&static_cast<Ti8Column*>(col)->GetData()[row]), sizeof(i8)};
            case ki16Column:
                return {reinterpret_cast<const char*>(&static_cast<Ti16Column*>(col)->GetData()[row]), sizeof(i16)};
            case ki32Column:
                return {reinterpret_cast<const char*>(&static_cast<Ti32Column*>(col)->GetData()[row]), sizeof(i32)};
            case ki64Column:
                return {reinterpret_cast<const char*>(&static_cast<Ti64Column*>(col)->GetData()[row]), sizeof(i64)};
            case ki128Column:
                return {reinterpret_cast<const char*>(&static_cast<Ti128Column*>(col)->GetData()[row]), sizeof(i128)};
            case kDoubleColumn:
                return {reinterpret_cast<const char*>(&static_cast<TDoubleColumn*>(col)->GetData()[row]), sizeof(ld)};
            case kDateColumn:
                return {reinterpret_cast<const char*>(&static_cast<TDateColumn*>(col)->GetData()[row]), sizeof(TDate)};
            case kTimestampColumn:
                return {reinterpret_cast<const char*>(&static_cast<TTimestampColumn*>(col)->GetData()[row]), sizeof(TTimestamp)};
            case kStringColumn: {
                auto& s = static_cast<TStringColumn*>(col)->GetData()[row];
                return {JStringBytes(s), s.size()};
            }
            default:
                return {nullptr, 0};
        }
    }

    inline ui64 ColsCnt() const { return col_idxs->size(); }
};

struct TKeyHasher {
    using is_transparent = void;
    ui64 operator()(const TStoredKey& k) const { return k.hash; }
    ui64 operator()(const TRowView& v) const { return v.hash; }
};

struct TKeyEq {
    using is_transparent = void;

    bool operator()(const TStoredKey& a, const TStoredKey& b) const {
        if (a.vals.size() != b.vals.size()) return false;
        for (ui64 i = 0; i < a.vals.size(); i++) {
            if (!(a.vals[i] == b.vals[i])) return false;
        }
        return true;
    }

    bool operator()(const TStoredKey& a, const TRowView& b) const {
        if (a.vals.size() != b.ColsCnt()) return false;
        for (ui64 i = 0; i < a.vals.size(); i++) {
            auto [p, sz] = b.BytesAt(i);
            if (a.vals[i].size() != sz) return false;
            if (std::memcmp(JStringBytes(a.vals[i]), p, sz) != 0) return false;
        }
        return true;
    }

    bool operator()(const TRowView& a, const TStoredKey& b) const { return (*this)(b, a); }
};

class TGroupBy : public ITableInput {
public:
    TGroupBy(TTableInputPtr jf_in, TGroupByQuery query, TAoQuery selects);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    const char* GetTypeName() const override { return "GroupBy"; }

private:
    TGroupByQuery group_q_;

    TTableInputPtr jf_in_;

    std::shared_ptr<IAoEngine> eng_;
    boost::unordered_flat_map<TStoredKey, ui64, TKeyHasher, TKeyEq> groups_;
    std::vector<i64> col_idxs_;
    std::vector<ui64> row_hashes_;
};

} // namespace JfEngine
