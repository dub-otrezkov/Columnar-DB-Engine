#pragma once

#include "table_node/types.h"

#include "utils/errors/errors.h"
#include "utils/csvio/csv_reader.h"

#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace JFEngine {

struct TRowScheme {
    std::string name_;
    std::string type_;
};

class ITableInput {
public:
    ITableInput(ui64 row_group_len = 1000);

    virtual Expected<void> GetColumnsScheme() = 0;
    virtual Expected<std::vector<std::shared_ptr<IColumn>>> ReadRowGroup() = 0;
    virtual void RestartDataRead() = 0;

    virtual std::vector<TRowScheme>& GetScheme() = 0;

    ui64 GetRowGroupLen() const;

protected:
    ui64 row_group_len_;
};

class TCSVTableInput : public ITableInput {
public:
    TCSVTableInput(std::istream& scheme_in, std::istream& data_in, ui64 row_group_len = 1000);

    Expected<void> GetColumnsScheme() override;
    Expected<std::vector<std::shared_ptr<IColumn>>> ReadRowGroup() override;
    // Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out, ui64 index) override;
    void RestartDataRead() override;

    std::vector<TRowScheme>& GetScheme() override;

private:
    TCSVReader scheme_in_;
    TCSVReader data_in_;
    std::vector<TRowScheme> scheme_;
};

class TJFTableInput : public ITableInput {
public:
    TJFTableInput(std::istream& jf_in);

    Expected<void> GetColumnsScheme() override;
    Expected<std::vector<std::shared_ptr<IColumn>>> ReadRowGroup() override;
    void RestartDataRead() override;

    std::vector<TRowScheme>& GetScheme() override;

private:
    std::istream& jf_in_;

    ui64 cols_cnt_;
    ui64 meta_start_;
    std::vector<ui64> blocks_pos_;
    std::vector<TRowScheme> scheme_;

    ui64 current_block_ = 0;
};

class TEngine {
    friend Expected<TEngine> MakeEngineFromCSV(std::istream& scheme, std::istream& data);
    friend Expected<TEngine> MakeEngineFromJF(std::istream& jf);
public:
    Expected<void> WriteSchemeToCSV(std::ostream& out);
    Expected<void> WriteDataToCSV(std::ostream& out);
    Expected<void> WriteTableToJF(std::ostream& out);

private:

    template <typename F>
    Expected<void> RunCommand(F func) {
        auto run = true;

        while (run) {
            auto [block_ptr, err] = in_->ReadRowGroup();

            if (err) {
                if (Is<EofErr>(err)) {
                    run = false;
                } else {
                    return err;
                }
            }
            if (!block_ptr) {
                continue;
            }
            auto block = *block_ptr;
            if (block.empty()) {
                continue;
            }
            auto res = func(std::move(block));
            if (!res) {
                return res.GetError();
            }
        }
        return nullptr;
    }

    Expected<void> Setup(std::unique_ptr<ITableInput>&& in);

    std::unique_ptr<ITableInput> in_;
};

Expected<TEngine> MakeEngineFromCSV(std::istream& scheme, std::istream& data);

Expected<TEngine> MakeEngineFromJF(std::istream& jf);

} // namespace JFEngine
