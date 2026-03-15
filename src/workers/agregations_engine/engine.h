#pragma once

#include "operators.h"

namespace JFEngine {

struct TGlobalAgregationQuery {
    std::vector<std::shared_ptr<IAgregation>> cols;

    TGlobalAgregationQuery Clone();
};

class TAgregationsEngine {
public:
    TAgregationsEngine(TGlobalAgregationQuery qry, bool groupby = false);

    Expected<void> ConsumeRowGroup(ITableInput* inp);
    Expected<std::vector<TColumnPtr>> ThrowRowGroup();

private:
    std::vector<std::shared_ptr<IAgregation>> cols_;
    bool is_groupby_;
};

} // namespace JFEngine