#pragma once

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include "columns/operators/filter.h"

namespace JFEngine {

struct TFilterOp {
    std::string column_name;
    EFilterType op;
    std::string value;
};

struct TFilterQuery {
    std::vector<TFilterOp> fils;
};

class TFilter : public ITableInput {
public:
    TFilter(std::shared_ptr<ITableInput> jf_in, TFilterQuery query);

    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
private:
    std::shared_ptr<ITableInput> jf_in_;
    std::unordered_map<std::string, ui64> name_to_i_;
    TFilterQuery query_;
};

} // namespace JFEngine
