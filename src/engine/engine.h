#pragma once

#include "workers/io/io.h"
#include "workers/selector/selector.h"

#include "table_node/types.h"

#include "utils/errors/errors.h"
#include "utils/csvio/csv_reader.h"

#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace JFEngine {

class TEngine {
    friend Expected<TEngine> MakeEngineFromCSV(
        std::istream& scheme,
        std::istream& data,
        ui64 row_group_size
    );
    friend Expected<TEngine> MakeEngineFromJF(std::istream& jf);
public:

    Expected<void> WriteSchemeToCSV(std::ostream& out);
    Expected<void> WriteDataToCSV(std::ostream& out);
    Expected<void> WriteTableToJF(std::ostream& out);

public:

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

Expected<TEngine> MakeEngineFromCSV(std::istream& scheme, std::istream& data, ui64 row_group_size = KRowGroupLen);

Expected<TEngine> MakeEngineFromJF(std::istream& jf);

Expected<TEngine> MakeSelectEngine(std::istream& jf, TSelectQuery query);

Expected<TEngine> MakeEngineFromWorker(std::unique_ptr<ITableInput>&& worker);

} // namespace JFEngine
