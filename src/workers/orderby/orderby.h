#pragma once

#include "csvio/csv_reader.h"
#include "columns/operators/order.h"
#include "workers/base.h"

#include <map>
#include <queue>

namespace JfEngine {

constexpr i64 kUnlimited = OMergeSort::kUnlimited;

struct TOrderByQuery {
    std::vector<std::string> cols;
    bool reverse = false;
    i64 limit = kUnlimited;
    i64 offset = 0;
};

class TOrderBy : public ITableInput {
public:
    TOrderBy(std::shared_ptr<ITableInput> jf_in, TOrderByQuery query);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    void SortRowGroup(std::vector<TColumnPtr>& rg, std::vector<TColumnPtr>& other);
    void MergeRowGroups(
        std::vector<TColumnPtr>& rg1,
        std::vector<TColumnPtr>& rg2
    );

    TOrderByQuery order_q_;
    
    std::shared_ptr<ITableInput> jf_in_;
};

} // namespace JfEngine
