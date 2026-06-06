#include "engine.h"

#include "columns/operators/operators.h"
#include "csvio/csv_writer.h"
#include "utils/logger/logger.h"

#include <cassert>

namespace JfEngine {

Expected<void> TEngine::Setup(TTableInputPtr in) {
    in_ = std::move(in);
    return in_->SetupColumnsScheme();
}

Expected<void> TEngine::WriteSchemeToCsv(IFileOutput* out) {
    TCsvWriter w(out);
    for (const auto& col : in_->GetScheme()) {
        w.WriteRow({col.name_, TColumnToStr(col.type_)});
    }
    return nullptr;
}

Expected<void> TEngine::WriteDataToCsv(IFileOutput* out) {
    TCsvWriter w(out);

    ui64 total_rows = 0;
    ui64 total_chars = 0;
    ui64 batch_idx = 0;

    auto f = [&](std::vector<TColumnPtr> block) -> Expected<void> {
        ui64 batch_chars = 0;
        ui64 batch_rows = block[0]->GetSize();
        for (ui64 i = 0; i < batch_rows; i++) {
            std::vector<std::string> row(block.size());
            for (ui64 j = 0; j < block.size(); j++) {
                row[j] = Do<OPrintIth>(block[j], i);
                batch_chars += row[j].size();
            }
            w.WriteRow(row);
        }
        total_rows += batch_rows;
        total_chars += batch_chars;
        batch_idx++;
        return nullptr;
    };

    auto res = RunCommand(f);
    return res;
}

Expected<void> TEngine::WriteTableToJf(IFileOutput* out) {
    std::vector<i64> poses;

    ui64 cols_cnt = 0;

    auto f = [&poses, out, &cols_cnt](std::vector<TColumnPtr> block) -> Expected<void> {
        std::vector<i64> col_poses;
        i64 sz = block[0]->GetSize();
        for (ui64 j = 0; j < block.size(); j++) {
            std::vector<std::string> row(block[0]->GetSize());
            col_poses.push_back(static_cast<i64>(out->TellPos()));
            auto bytes = Do<OJfPrintOpt>(block[j]);
            const char* p = bytes.data();
            out->Write(p, bytes.size());
        }

        cols_cnt = block.size();

        for (auto pos : col_poses) {
            PutI64(out, pos);
        }

        PutI64(out, sz);
        poses.push_back(static_cast<i64>(out->TellPos()));

        return nullptr;
    };

    RunCommand(f);

    auto meta_start = static_cast<i64>(out->TellPos());
    PutI64(out, in_->GetRowGroupLen());
    PutI64(out, cols_cnt);
    PutI64(out, poses.size());
    for (auto i : poses) {
        PutI64(out, i);
    }
    auto err = WriteSchemeToCsv(out);

    PutI64(out, meta_start);

    return err;
}

Expected<TEngine> MakeEngineFromCsv(
    IFileInput* scheme,
    IFileInput* data,
    ui64 row_group_size
) {
    TEngine eng;
    auto err = eng.Setup(std::make_shared<TCsvTableInput>(scheme, data, row_group_size));
    if (!err) {
        return err.GetError();
    }
    return std::move(eng);
}

Expected<TEngine> MakeEngineFromJf(IFileInput* jf) {
    TEngine eng;
    auto err = eng.Setup(std::make_shared<TJfTableInput>(jf));
    if (err.HasError()) {
        return err.GetError();
    }
    return std::move(eng);
}

Expected<TEngine> MakeEngineFromWorker(TTableInputPtr worker) {
    TEngine eng;
    auto err = eng.Setup(worker);
    if (err.HasError()) {
        return err.GetError();
    }
    return std::move(eng);
}

}
