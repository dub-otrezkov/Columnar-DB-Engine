#pragma once

#include "csvio/csv_reader.h"
#include "utils/mmap_input/mmap_input.h"
#include "workers/base.h"

#include <unordered_set>

namespace JfEngine {

class TCsvTableInput : public ITableInput {
public:
    TCsvTableInput(IFileInput* scheme_in, IFileInput* data_in, ui64 row_group_len = kRowGroupLen) :
        scheme_in_(scheme_in),
        data_in_(data_in),
        csv_data_(data_in_)
    {
    }

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    const char* GetTypeName() const override {
        return "CsvTableInput";
    }

private:
    IFileInput* scheme_in_;
    IFileInput* data_in_;
    TCsvOptimizedReader csv_data_;
};

class TJfTableInput : public ITableInput {
public:
    virtual ~TJfTableInput() = default;

    TJfTableInput(IFileInput* jf_in) {
        jf_in_ = jf_in;
    }

    Expected<TColumnPtr> Finalize(ui64 column, const std::vector<ui64>& idxs);
    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    Expected<TColumnPtr> ReadColumn(const std::string& name) override;
    Expected<TColumnPtr> ReadIthColumn(i64 i) override;
    Expected<TColumnPtr> ReadMinMax(i64 i) override;

    void MoveCursor() override;
    void Reset() override;
    const char* GetTypeName() const override {
        return "JfTableInput";
    }

private:
    Expected<TColumnPtr> ReadColumnFromMem(ui64 column, ui64 rg_num);

protected:

    IFileInput* jf_in_;

    ui64 cols_cnt_ = 0;
    i64  meta_start_ = 0;

    // [block][col] → byte offset in file
    std::vector<std::vector<i64>>  block_col_poses_;
    // [block][col] → row count (for lazy materialization)
    std::vector<std::vector<ui64>> block_col_sizes_;

    ui64 current_block_ = 0;
};

class TJfNeccessaryOnly : public TJfTableInput {
public:
    TJfNeccessaryOnly(IFileInput* jf_in, std::unordered_set<std::string> referenced = {});

    std::vector<TRowScheme>& GetScheme() override;
    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;
    Expected<TColumnPtr> ReadMinMax(i64 i) override {
        return TJfTableInput::ReadMinMax(cols_.at(i));
    }
    Expected<TColumnPtr> ReadMinMax(const std::string& name) override {
        for (ui64 j = 0; j < new_scheme_.size(); j++) {
            if (new_scheme_[j].name_ == name) {
                return ReadMinMax(static_cast<i64>(j));
            }
        }
        return MakeError<EError::NoSuchColumnsErr>(name);
    }
    const char* GetTypeName() const override {
        return "JfNeccessaryOnly";
    }

private:
    std::unordered_set<std::string> referenced_;
    std::vector<ui64> cols_;

    std::vector<TRowScheme> new_scheme_;
};

} // JfEngine
