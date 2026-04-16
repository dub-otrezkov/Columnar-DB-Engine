#pragma once

#include "narrow_io.h"

#include "workers/ao_engine/engine.h"

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include "columns/operators/filter.h"
#include "utils/faster_hashmap/hashmap.h"

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
    TGroupBy(std::shared_ptr<ITableInput> jf_in, TGroupByQuery query, TAoQuery selects);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    TGroupByQuery group_q_;
    TAoQuery agr_q_;
    
    std::shared_ptr<ITableInput> jf_in_;

    std::shared_ptr<IAoEngine> gc_eng;

    struct TGroup {
        std::shared_ptr<IAoEngine> eng;
        TNarrowTableInput io;

        TGroup(std::vector<TRowScheme>& scheme, TAoQuery agr_q) :
            eng(MakeAoEngine(std::move(agr_q))),
            io(scheme)
        {}
    };

    struct VectorStringHashed {
        StringVector vals;
        ui64 hash;

        VectorStringHashed(StringVector vals_ = {}) : vals(std::move(vals_)), hash(0) {
            const char* data = vals.data();
            size_t size = vals.data_size();

            while (size >= 8) {
                ui64 chunk;
                std::memcpy(&chunk, data, 8);
                hash = _mm_crc32_u64(hash, chunk);
                data += 8;
                size -= 8;
            }
            while (size > 0) {
                hash = _mm_crc32_u8(static_cast<unsigned int>(hash), *data);
                data++;
                size--;
            }

            hash ^= hash >> 33;
            hash *= 0xff51afd7ed558ccdULL;
            hash ^= hash >> 33;
        }

        bool operator==(const VectorStringHashed& other) const {
            if (hash != other.hash) {
                return false;
            }
            return vals == other.vals;
        }
    };

    struct VectorStringHasher {
        inline ui64 operator()(const VectorStringHashed& a) const {
            return a.hash;
        }
    };

    using THashMap = std::unordered_map<VectorStringHashed, TGroup, VectorStringHasher>;
    static THashMap groups_;
};

} // namespace JfEngine
