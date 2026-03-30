#pragma once

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include <map>
#include <queue>

namespace JFEngine {

constexpr ui64 kUnlimited = -1;

struct TOrderByQuery {
    std::vector<std::string> cols;
    bool reverse = false;
    ui64 limit = kUnlimited;
};

class TOrderBy : public ITableInput {
public:
    TOrderBy(std::shared_ptr<ITableInput> jf_in, TOrderByQuery query);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    void SortRowGroup(std::vector<TColumnPtr>& rg);
    void SortRowGroup(std::vector<TColumnPtr>& rg, ui64 column);
    void MergeRowGroups(
        std::vector<TColumnPtr>& rg1,
        std::vector<TColumnPtr>& rg2
    );

    TOrderByQuery order_q_;
    
    std::shared_ptr<ITableInput> jf_in_;
};

} // namespace JFEngine
