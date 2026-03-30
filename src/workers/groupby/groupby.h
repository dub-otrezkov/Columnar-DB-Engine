#pragma once

#include "narrow_io.h"

#include "workers/agregations_engine/engine.h"

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include "columns/operators/filter.h"

#include <map>

namespace JFEngine {

constexpr ui64 kUnlimited = -1;

struct TGroupByQuery {
    std::vector<std::shared_ptr<IAgregation>> cols;
    ui64 limit = kUnlimited;
    bool is_id = false;
};

class TGroupBy : public ITableInput {
public:
    TGroupBy(std::shared_ptr<ITableInput> jf_in, TGroupByQuery query, TGlobalAgregationQuery selects);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    TGroupByQuery group_q_;
    TGlobalAgregationQuery agr_q_;
    
    std::shared_ptr<ITableInput> jf_in_;
    std::vector<TRowScheme> scheme_;

    TAgregationsEngine gc_eng;

    struct TGroup {
        TAgregationsEngine eng;
        TNarrowTableInput io;

        TGroup(std::vector<TRowScheme>& scheme, TGlobalAgregationQuery agr_q) :
            eng(std::move(agr_q), /*groupby=*/true),
            io(scheme)
        {}
    };

    std::map<std::vector<std::string>, TGroup> groups_;
};

} // namespace JFEngine
