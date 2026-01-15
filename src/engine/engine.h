#pragma once

#include "nodes_factory.h"

#include "utils/errors/errors.h"
#include "utils/csvio/csv_reader.h"

#include "table_node/types.h"

#include <memory>
#include <string>
#include <vector>

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

class ITableInput {
public:
    ITableInput(ui64 row_group_len = 1000);

    virtual IError* GetColumnsScheme(std::vector<TRowScheme>& out) = 0;
    virtual IError* ReadRowGroup(std::vector<std::vector<std::string>>& out) = 0;
    virtual void RestartDataRead() = 0;

protected:
    ui64 row_group_len_;
};

class TCSVTableInput : public ITableInput {
public:
    TCSVTableInput(std::istream& scheme_in, std::istream& data_in, ui64 row_group_len = 1000);

    IError* GetColumnsScheme(std::vector<TRowScheme>& out) override;
    IError* ReadRowGroup(std::vector<std::vector<std::string>>& out) override;
    void RestartDataRead() override;

private:
    TCSVReader scheme_in_;
    TCSVReader data_in_;
};

class TJFTableInput : public ITableInput {
public:
    TJFTableInput(std::istream& jf_in);

    IError* GetColumnsScheme(std::vector<TRowScheme>& out) override;
    IError* ReadRowGroup(std::vector<std::vector<std::string>>& out) override;
    void RestartDataRead() override;

private:
    std::istream& jf_in_;
};

class TEngine {
    friend std::pair<std::shared_ptr<TEngine>, IError*> MakeEngineFromCSV(std::istream& scheme, std::istream& data);
    friend std::pair<std::shared_ptr<TEngine>, IError*> MakeEngineFromJF(std::istream& jf);
public:
    IError* WriteSchemeToCSV(std::ostream& out);
    IError* WriteDataToCSV(std::ostream& out);
    IError* WriteTableToJF(std::ostream& out);

private:

    template <typename F>
    IError* RunCommand(F func) {
        auto run = true;

        while (run) {
            std::vector<std::vector<std::shared_ptr<ITableNode>>> block;

            auto err = ReadRowGroup(block);

            if (err) {
                if (Is<EofErr>(err)) {
                    run = false;
                    delete err;
                } else {
                    return err;
                }
            }
            if (block.empty()) {
                continue;
            }

            err = func(std::move(block));
            if (err) {
                return err;
            }
        }
        return nullptr;
    }

    IError* Setup(std::unique_ptr<ITableInput>&& in);

    IError* ReadRowGroup(std::vector<std::vector<std::shared_ptr<ITableNode>>>& out);

    std::unique_ptr<ITableInput> in_;
    std::vector<TRowScheme> cols_;
};

std::pair<std::shared_ptr<TEngine>, IError*> MakeEngineFromCSV(std::istream& scheme, std::istream& data);

std::pair<std::shared_ptr<TEngine>, IError*> MakeEngineFromJF(std::istream& jf);
