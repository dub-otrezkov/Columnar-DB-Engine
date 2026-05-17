#pragma once

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include <iostream>
#include <unordered_set>

namespace JfEngine {

class TCsvTableInput : public ITableInput {
public:
    TCsvTableInput(std::shared_ptr<std::istream> scheme_in, std::shared_ptr<std::istream> data_in, ui64 row_group_len = kRowGroupLen) :
        scheme_in_(std::move(scheme_in)),
        data_in_(std::move(data_in)),
        csv_data_(*data_in_)
    {
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    const char* GetTypeName() const override { return "CsvTableInput"; }

private:
    std::shared_ptr<std::istream> scheme_in_;
    std::shared_ptr<std::istream> data_in_;
    TCsvOptimizedReader csv_data_;
};

class TJfTableInput : public ITableInput {
public:
    virtual ~TJfTableInput() = default;

    TJfTableInput(std::shared_ptr<std::istream> jf_in) {
        jf_in_ = jf_in;
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    Expected<TColumnPtr> ReadColumn(const std::string& name) override;
    Expected<TColumnPtr> ReadIthColumn(i64 i) override;

    void MoveCursor() override;
    void Reset() override;
    const char* GetTypeName() const override { return "JfTableInput"; }

protected:

    std::shared_ptr<std::istream> jf_in_;

    std::optional<std::vector<ui64>> poses_of_cols_;

    ui64 cols_cnt_;
    ui64 meta_start_;
    std::vector<ui64> blocks_pos_;

    ui64 current_block_ = 0;
};

class TJfNeccessaryOnly : public TJfTableInput {
public:
    TJfNeccessaryOnly(std::shared_ptr<std::istream> jf_in, std::unordered_set<std::string> referenced = {});

    std::vector<TRowScheme>& GetScheme() override;
    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    const char* GetTypeName() const override { return "JfNeccessaryOnly"; }

private:
    std::unordered_set<std::string> referenced_;
    std::vector<ui64> cols_;

    std::vector<TRowScheme> new_scheme_;
};

} // JfEngine
