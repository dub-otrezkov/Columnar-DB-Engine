#pragma once

#include "workers/base.h"

namespace JfEngine {

class TNarrowTableInput : public ITableInput {
public:
    void Update(std::vector<TRowScheme>& scheme);

    Expected<void> SetupColumnsScheme() override;

    void MoveCursor() override;

    void UploadRowGroup(std::vector<TColumnPtr>& row_group, std::vector<ui64>& row_i);
    void UploadRowGroup(std::vector<TColumnPtr>& row_group, std::pair<i64, i64>& row_i);

    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    std::shared_ptr<std::vector<TColumnPtr>> buf_;
};

} // namespace JfEngine