#pragma once

#include "workers/agregations_engine/engine.h"
#include "workers/io/io.h"

#include <iostream>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>

namespace JFEngine {

class TAgregator : public ITableInput {
public:
    TAgregator(std::shared_ptr<ITableInput> jf_in); // return full table
    TAgregator(std::shared_ptr<ITableInput> jf_in, TGlobalAgregationQuery query);

    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
private:
    std::shared_ptr<ITableInput> jf_in_;
    std::vector<TRowScheme> scheme_;

    TAgregationsEngine eng_;

    ui64 cols_cnt_ = 0;
    bool blocker_ = false;
    bool is_all_ = false;
};

} // namespace JFEngine