#pragma once

#include "columns/types/types.h"
#include "csvio/csv_reader.h"
#include "utils/errors/errors.h"
#include "utils/mmap_input/mmap_input.h"
#include "workers/io/io.h"

#include <memory>
#include <string>
#include <vector>

namespace JfEngine {

class TEngine {
    friend Expected<TEngine> MakeEngineFromCsv(
        IFileInput* scheme,
        IFileInput* data,
        ui64 row_group_size
    );
    friend Expected<TEngine> MakeEngineFromJf(IFileInput* jf);
public:

    Expected<void> WriteSchemeToCsv(IFileOutput* out);
    Expected<void> WriteDataToCsv(IFileOutput* out);
    Expected<void> WriteTableToJf(IFileOutput* out);

public:

    template <typename F>
    Expected<void> RunCommand(F func) {

        auto run = true;

        for (; run; in_->MoveCursor()) {
            auto [block, err] = in_->ReadRowGroup();

            if (err != EError::NoError) {
                if (Is<EError::EofErr>(err)) {
                    run = false;
                } else {
                    return err;
                }
            }
            if (block.empty()) {
                continue;
            }

            if (block[0]->GetSize() == 0) {
                continue;
            }
            auto res = func(std::move(block));
            TStringHeap::Free();
            if (!res) {
                return res.GetError();
            }
        }
        return nullptr;
    }

    Expected<void> Setup(TTableInputPtr in);

    TTableInputPtr in_;
};

Expected<TEngine> MakeEngineFromCsv(
    IFileInput* scheme,
    IFileInput* data,
    ui64 row_group_size = kRowGroupLen
);

Expected<TEngine> MakeEngineFromJf(IFileInput* jf);

Expected<TEngine> MakeEngineFromWorker(TTableInputPtr worker);

}
