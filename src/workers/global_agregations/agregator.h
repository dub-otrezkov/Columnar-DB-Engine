#pragma once

#include "workers/ao_engine/engine.h"
#include "workers/io/io.h"

#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace JfEngine {

class TAgregator : public ITableInput {
public:
    TAgregator(TTableInputPtr jf_in); // return full table
    TAgregator(TTableInputPtr jf_in, TAoQuery query);

    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    void MoveCursor() override;
    const char* GetTypeName() const override { return "Agregator"; }

private:
    TTableInputPtr jf_in_;

    std::shared_ptr<IAoEngine> eng_;

    ui64 cols_cnt_ = 0;
    bool is_all_ = false;
};

} // namespace JfEngine