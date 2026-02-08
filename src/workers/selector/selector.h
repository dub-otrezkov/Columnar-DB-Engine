#pragma once

#include "workers/base.h"
#include "workers/io/io.h"

#include <iostream>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>

namespace JFEngine {

struct TSelectQuery {
    std::vector<std::string> rows;
    std::unordered_map<std::string, std::string> aliases = {};
};

class TSelector : public ITableInput {
public:
    TSelector(std::shared_ptr<TJFTableInput> jf_in, TSelectQuery query);

    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
private:
    std::shared_ptr<TJFTableInput> jf_in_;
    std::vector<TRowScheme> scheme_;
    std::unordered_map<std::string, std::string> aliases_;
    std::unordered_map<std::string, std::string> unaliases_;
};

} // namespace JFEngine