#pragma once

#include "workers/base.h"

namespace JFEngine {

class TNarrowTableInput : public ITableInput {
public:
    TNarrowTableInput(std::vector<TRowScheme>& scheme);

    Expected<void> SetupColumnsScheme() override;
    std::vector<TRowScheme>& GetScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
    Expected<IColumn> ReadColumn(const std::string& name) override;

    void UploadRowGroup(std::vector<TColumnPtr>& row_group, ui64 row_i);

    ui64 GetRowGroupLen() const;

private:
    std::unordered_map<std::string, ui64> name_to_i_;
    std::vector<TColumnPtr> buf_;
};

} // namespace JFEngine