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
    const char* GetTypeName() const override { return "GroupBy"; }

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

    struct VectorStringHasher {
        using is_transparent = void;

        ui64 operator()(const std::vector<JString>& a) const {
            ui64 h = 0;
            for (ui64 i = 0; i < a.size(); i++) {
                h ^= hash_value(a.at(i)) ^ (h << 13);
            }
            return h;
        }
    };

    template<typename T>
    using THashMap = boost::unordered_flat_map<
        std::vector<JString>, T,
        VectorStringHasher
    >;

    boost::unordered_flat_map<std::string, ui64> name_to_i_in_;
    std::vector<std::string> group_col_names_;
    THashMap<std::vector<ui64>> keys;
    THashMap<TGroup> groups_;
    std::optional<TNarrowTableInput> inp_;
    std::vector<StringVector> printed_;
    std::vector<ui64> row_hashes_;
};

} // namespace JfEngine