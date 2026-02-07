#pragma once

#include "errors.h"

#include "utils/csvio/csv_reader.h"
#include "workers/base.h"

#include <iostream>

namespace JFEngine {

class TCSVTableInput : public ITableInput {
public:
    TCSVTableInput(std::shared_ptr<std::istream> scheme_in, std::shared_ptr<std::istream> data_in, ui64 row_group_len = KRowGroupLen) {
        scheme_in_ = std::move(scheme_in);
        data_in_ = std::move(data_in);
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
    std::vector<TRowScheme>& GetScheme() override;

private:
    std::shared_ptr<std::istream> scheme_in_;
    std::shared_ptr<std::istream> data_in_;
    std::vector<TRowScheme> scheme_;
};

class TJFTableInput : public ITableInput {
public:
    TJFTableInput(std::shared_ptr<std::istream> jf_in) {
        jf_in_ = jf_in;
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> ReadRowGroup() override;
    Expected<TColumnPtr> ReadColumn(const std::string& name) override;
    std::vector<TRowScheme>& GetScheme() override;

    void MoveCursor(i64 delta);

private:
    Expected<TColumnPtr> ReadIthColumn(ui64 i);

    std::shared_ptr<std::istream> jf_in_;

    ui64 cols_cnt_;
    ui64 meta_start_;
    std::vector<ui64> blocks_pos_;
    std::vector<TRowScheme> scheme_;

    ui64 current_block_ = 0;
};

} // JFEngine
