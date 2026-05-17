#pragma once

#include "columns/operators/filter.h"
#include "csvio/csv_reader.h"
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
    std::vector<std::string> cols;
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

    TTableInputPtr jf_in_;

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

    std::shared_ptr<IAoEngine> eng_;
    THashMap<ui64> groups_;
};

} // namespace JfEngine