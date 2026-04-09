#pragma once

#include "workers/io/io.h"

#include "columns/types/types.h"

#include "utils/errors/errors.h"
#include "csvio/csv_reader.h"

#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace JfEngine {

class TEngine {
    friend Expected<TEngine> MakeEngineFromCsv(
        std::shared_ptr<std::istream> scheme,
        std::shared_ptr<std::istream> data,
        ui64 row_group_size
    );
    friend Expected<TEngine> MakeEngineFromJf(std::shared_ptr<std::istream> jf);
public:

    Expected<void> WriteSchemeToCsv(std::ostream& out);
    Expected<void> WriteDataToCsv(std::ostream& out);
    Expected<void> WriteTableToJf(std::ostream& out);

public:

    template <typename F>
    Expected<void> RunCommand(F func) {

        auto run = true;

        for (; run; in_->MoveCursor()) {
            auto [block_ptr, err] = in_->ReadRowGroup();

            if (err != EError::NoError) {
                if (Is<EError::EofErr>(err)) {
                    run = false;
                } else {
                    std::cout << "engine run command err: " << err << std::endl;
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

            if (block[0]->GetSize() == 0) {
                continue;
            }
            auto res = func(std::move(block));
            if (!res) {
                return res.GetError();
            }
        }
        return nullptr;
    }

    Expected<void> Setup(std::shared_ptr<ITableInput> in);

    std::shared_ptr<ITableInput> in_;
};

Expected<TEngine> MakeEngineFromCsv(std::shared_ptr<std::istream> scheme, std::shared_ptr<std::istream> data, ui64 row_group_size = kRowGroupLen);

Expected<TEngine> MakeEngineFromJf(std::shared_ptr<std::istream> jf);

Expected<TEngine> MakeEngineFromWorker(std::shared_ptr<ITableInput> worker);

} // namespace JfEngine
