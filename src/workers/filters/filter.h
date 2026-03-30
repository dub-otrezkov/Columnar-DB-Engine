#pragma once

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include "columns/operators/filter.h"

namespace JFEngine {

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
    TFilter(std::shared_ptr<ITableInput> jf_in, TFilterQuery query);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    void MoveCursor(i64 delta);
private:
    std::shared_ptr<ITableInput> jf_in_;
    TFilterQuery query_;
};

} // namespace JFEngine
