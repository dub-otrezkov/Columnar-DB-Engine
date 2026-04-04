#pragma once

#include "operators.h"

namespace JfEngine {

struct TGlobalAgregationQuery {
    std::vector<std::shared_ptr<IAgregation>> cols;
    std::vector<std::pair<ui64, std::string>> aliases;

    TGlobalAgregationQuery Clone();
};

class TAgregationsEngine {
public:
    TAgregationsEngine() = default;
    TAgregationsEngine(TGlobalAgregationQuery qry, bool groupby = false);

    std::vector<std::string> GetNames();

    Expected<void> ConsumeRowGroup(ITableInput* inp);
    Expected<std::vector<TColumnPtr>> ThrowRowGroup();

private:
    std::vector<std::shared_ptr<IAgregation>> cols_;
    std::vector<std::pair<ui64, std::string>> aliases_;
    bool is_groupby_;
    bool total_ = false;
};

} // namespace JfEngine