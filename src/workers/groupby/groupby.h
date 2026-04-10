#pragma once

#include "narrow_io.h"

#include "workers/ao_engine/engine.h"

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include "columns/operators/filter.h"
#include "utils/faster_hashmap/hashmap.h"

#include <map>

namespace JfEngine {

constexpr ui64 kUnlimited = -1;

struct TGroupByQuery {
    std::vector<std::shared_ptr<IOa>> cols;
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

    struct VectorStringHasher {
        std::size_t operator()(const std::vector<std::string>& v) const {
            std::size_t seed = 2929929;
            std::hash<std::string> hasher;
            
            for (const auto& s : v) {
                seed ^= hasher(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            
            return seed;
        }
    };

    std::unordered_map<std::vector<std::string>, TGroup, VectorStringHasher> groups_;
};

} // namespace JfEngine
