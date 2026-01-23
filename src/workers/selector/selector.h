#pragma once

#include "workers/base.h"
#include "workers/io/io.h"

#include <iostream>
#include <vector>

namespace JFEngine {

class TSelector : public ITableInput {
public:
    TSelector(std::istream& jf_in, const std::vector<std::string>& rows);
    
    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
    void RestartDataRead() override;
private:
    std::unique_ptr<TJFTableInput> jf_in_;
    std::vector<TRowScheme> scheme_;
};

} // namespace JFEngine