#pragma once

#include "columns/operators/filter.h"
#include "csvio/csv_reader.h"
#include "narrow_io.h"
#include "utils/faster_hashmap/hashmap.h"
#include "workers/ao_engine/engine.h"
#include "workers/base.h"

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_set.hpp>
#include <nmmintrin.h>

#include <map>

namespace JfEngine {

constexpr ui64 kUnlimited = -1;

struct TGroupByQuery {
    std::vector<std::unique_ptr<IOa>> cols;
    ui64 limit = kUnlimited;
    bool is_id = false;
};

class TGroupBy : public ITableInput {
public:
    TGroupBy(TTableInputPtr jf_in, TGroupByQuery query, TAoQuery selects);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    TGroupByQuery group_q_;
    TAoQuery agr_q_;

    TTableInputPtr jf_in_;
    std::shared_ptr<IAoEngine> gc_eng;

    struct TGroup {
        std::shared_ptr<IAoEngine> eng;

        TGroup() = default;
        TGroup(TAoQuery agr_q) :
            eng(MakeAoEngine(std::move(agr_q)))
        {}
    };

    // Хранится в map — владеет данными
    struct VectorStringHashed {
        StringVector vals;
        ui64 hash;

        VectorStringHashed() : hash(0) {}

        VectorStringHashed(StringVector vals_, ui64 precomputed_hash)
            : vals(std::move(vals_)), hash(precomputed_hash) {}

        explicit VectorStringHashed(StringVector vals_)
            : vals(std::move(vals_)), hash(0)
        {
            const char* data = vals.data();
            size_t size = vals.data_size();
            while (size >= 8) {
                ui64 chunk;
                std::memcpy(&chunk, data, 8);
                hash = _mm_crc32_u64(hash, chunk);
                data += 8; size -= 8;
            }
            while (size > 0) {
                hash = _mm_crc32_u8(static_cast<unsigned int>(hash), *data++);
                size--;
            }
            const ui64* offs = vals.offsets_data();
            size_t offs_size = vals.size() * sizeof(ui64);
            while (offs_size >= 8) {
                ui64 chunk;
                std::memcpy(&chunk, offs, 8);
                hash = _mm_crc32_u64(hash, chunk);
                offs = reinterpret_cast<const ui64*>(
                    reinterpret_cast<const char*>(offs) + 8
                );
                offs_size -= 8;
            }
            hash ^= hash >> 33;
            hash *= 0xff51afd7ed558ccdULL;
            hash ^= hash >> 33;
        }

        bool operator==(const VectorStringHashed& other) const {
            if (hash != other.hash) return false;
            return vals == other.vals;
        }
    };

    // Лёгкий view на строку батча — не владеет данными, не аллоцирует
    struct RowView {
        const std::vector<TColumnPtr>* cols;
        ui64 row;
        ui64 hash;
    };

    struct VectorStringHasher {
        using is_transparent = void;

        ui64 operator()(const VectorStringHashed& a) const {
            return a.hash;
        }
        ui64 operator()(const RowView& r) const {
            return r.hash;
        }
    };

    struct VectorStringEqual {
        using is_transparent = void;

        bool operator()(const VectorStringHashed& a, const VectorStringHashed& b) const {
            return a == b;
        }

        static bool Cmp(const RowView& view, const VectorStringHashed& key) {
            if (view.hash != key.hash) return false;
            if (view.cols->size() != key.vals.size()) return false;
            for (ui64 c = 0; c < view.cols->size(); c++) {
                if (!Do<OEqualRow>((*view.cols)[c], view.row, key.vals, c)) {
                    return false;
                }
            }
            return true;
        }

        bool operator()(const RowView& view, const VectorStringHashed& key) const {
            if (view.hash != key.hash) return false;
            if (view.cols->size() != key.vals.size()) return false;
            for (ui64 c = 0; c < view.cols->size(); c++) {
                if (!Do<OEqualRow>((*view.cols)[c], view.row, key.vals, c)) {
                    return false;
                }
            }
            return true;
        }

        bool operator()(const VectorStringHashed& key, const RowView& view) const {
            return (*this)(view, key);
        }
    };

    static inline ui64 Finalize(ui64 h) {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        return h;
    }

    template<typename T>
    using THashMap = boost::unordered_flat_map<
        VectorStringHashed, T,
        VectorStringHasher,
        VectorStringEqual
    >;

    // THashMap<std::vector<ui64>> keys;
    THashMap<TGroup> groups_;
    std::optional<TNarrowTableInput> inp_;
    std::vector<StringVector> printed_;
    std::vector<ui64> row_hashes_;
};

} // namespace JfEngine