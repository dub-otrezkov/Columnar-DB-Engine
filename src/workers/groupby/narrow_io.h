#pragma once

#include "workers/base.h"

namespace JFEngine {

class TNarrowTableInput : public ITableInput {
public:
    TNarrowTableInput(std::vector<TRowScheme>& scheme);

    Expected<void> SetupColumnsScheme() override;

    void MoveCursor(i64 delta) override;

    void UploadRowGroup(std::vector<TColumnPtr>& row_group, ui64 row_i);

    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    std::shared_ptr<std::vector<TColumnPtr>> buf_;
};

} // namespace JFEngine