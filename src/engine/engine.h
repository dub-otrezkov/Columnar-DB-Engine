#pragma once

#include "nodes_factory.h"

#include "utils/errors/errors.h"
#include "utils/csvio/csv_reader.h"

#include "table_node/types.h"

#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace JFEngine {

class IncorrrectFileErr : public IError {
public:
    IncorrrectFileErr(std::string message = "bad input") : message_(std::move(message)) {
    }

    std::string Print() const override {
        return message_;
    }

private:
    std::string message_;
};

class UnimplementedErr : public IError {
public:

    std::string Print() const override {
        return "unimplemented";
    }
};

class ITableInput {
public:
    ITableInput(ui64 row_group_len = 1000);

    virtual Expected<void> GetColumnsScheme(std::vector<TRowScheme>& out) = 0;
    virtual Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out) = 0;
    virtual Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out, ui64 index) = 0;
    virtual void RestartDataRead() = 0;


protected:
    ui64 row_group_len_;
};

class TCSVTableInput : public ITableInput {
public:
    TCSVTableInput(std::istream& scheme_in, std::istream& data_in, ui64 row_group_len = 1000);

    Expected<void> GetColumnsScheme(std::vector<TRowScheme>& out) override;
    Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out) override;
    Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out, ui64 index) override;
    void RestartDataRead() override;

private:
    TCSVReader scheme_in_;
    TCSVReader data_in_;
};

class TJFTableInput : public ITableInput {
public:
    TJFTableInput(std::istream& jf_in);

    Expected<void> GetColumnsScheme(std::vector<TRowScheme>& out) override;
    Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out) override;
    Expected<void> ReadRowGroup(std::vector<std::vector<std::string>>& out, ui64 index) override;
    void RestartDataRead() override;

private:
    std::istream& jf_in_;
    std::vector<ui64> blocks_pos;
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
            std::vector<std::shared_ptr<IColumn>> block;
            {
                auto res = ReadRowGroup(block);

                if (res.HasError()) {
                    if (Is<EofErr>(res.GetError())) {
                        run = false;
                    } else {
                        return res.GetError();
                    }
                }
                if (block.empty()) {
                    continue;
                }
            }
            {
                auto res = func(std::move(block));
                if (!res) {
                    return res.GetError();
                }
            }
        }
        return nullptr;
    }

    Expected<void> Setup(std::unique_ptr<ITableInput>&& in);

    Expected<void> ReadRowGroup(std::vector<std::shared_ptr<IColumn>>& out);

    std::unique_ptr<ITableInput> in_;
    std::vector<TRowScheme> cols_;
};

Expected<TEngine> MakeEngineFromCSV(std::istream& scheme, std::istream& data);

Expected<TEngine> MakeEngineFromJF(std::istream& jf);

} // namespace JFEngine
