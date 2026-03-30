#pragma once

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include <iostream>

namespace JFEngine {

class TCSVTableInput : public ITableInput {
public:
    TCSVTableInput(std::shared_ptr<std::istream> scheme_in, std::shared_ptr<std::istream> data_in, ui64 row_group_len = kRowGroupLen) :
        scheme_in_(std::move(scheme_in)),
        data_in_(std::move(data_in)),
        csv_data_(*data_in_)
    {
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    std::shared_ptr<std::istream> scheme_in_;
    std::shared_ptr<std::istream> data_in_;
    TCSVOptimizedReader csv_data_;
};

class TJFTableInput : public ITableInput {
public:
    TJFTableInput(std::shared_ptr<std::istream> jf_in) {
        jf_in_ = jf_in;
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    Expected<IColumn> ReadColumn(const std::string& name) override;

    void MoveCursor(i64 delta) override;
    void Reset() override;
    ui64 GetGroupsCount() const override;

private:
    Expected<IColumn> ReadIthColumn(ui64 i);

    std::shared_ptr<std::istream> jf_in_;

    ui64 cols_cnt_;
    ui64 meta_start_;
    std::vector<ui64> blocks_pos_;

    ui64 current_block_ = 0;
};

} // JFEngine
