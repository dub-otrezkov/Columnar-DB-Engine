#pragma once

#include "operators.h"

#include "workers/io/io.h"

#include <iostream>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>

namespace JFEngine {

struct TGlobalAgregationQuery {
    std::vector<std::shared_ptr<IAgregation>> cols;
};

class TAgregator : public ITableInput {
public:
    TAgregator(std::shared_ptr<ITableInput> jf_in, TGlobalAgregationQuery query);

    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
private:
    std::shared_ptr<ITableInput> jf_in_;
    std::vector<TRowScheme> scheme_;

    TGlobalAgregationQuery query_;
};

} // namespace JFEngine