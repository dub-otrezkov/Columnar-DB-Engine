#pragma once

#include "columns/operators/filter.h"
#include "csvio/csv_reader.h"
#include "workers/base.h"

namespace JfEngine {

struct TFilterOp {
    std::string column_name;
    EFilterType op;
    std::string value;

    std::optional<std::vector<std::string>> args_for_in = std::nullopt;
};

struct TFilterQuery {
    std::vector<TFilterOp> fils;
};

class TFilter : public ITableInput {
public:
    TFilter(TTableInputPtr jf_in, TFilterQuery query);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    void MoveCursor() override;
private:
    TTableInputPtr jf_in_;
    TFilterQuery query_;
};

} // namespace JfEngine
