#include "engine.h"

#include "columns/operators/operators.h"
#include "csvio/csv_writer.h"
#include "utils/logger/logger.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace JfEngine {

Expected<void> TEngine::Setup(TTableInputPtr in) {
    in_ = std::move(in);
    return in_->SetupColumnsScheme();
}

Expected<void> TEngine::WriteSchemeToCsv(std::ostream& out) {
    TCsvWriter w(out);
    for (const auto& col : in_->GetScheme()) {
        w.WriteRow({col.name_, TColumnToStr(col.type_)});
    }
    return nullptr;
}

Expected<void> TEngine::WriteDataToCsv(std::ostream& out) {
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
        // JF_LOG(this, "batch=" << batch_idx
        //     << " rows=" << batch_rows
        //     << " chars=" << batch_chars
        //     << " total_rows=" << total_rows
        //     << " total_chars=" << total_chars);
        batch_idx++;
        return nullptr;
    };

    auto res = RunCommand(f);
    // JF_LOG(this, "DONE total_rows=" << total_rows
    //     << " total_chars=" << total_chars
    //     << " batches=" << batch_idx);
    return res;
}

Expected<void> TEngine::WriteTableToJf(std::ostream& out) {
    std::vector<i64> poses;

    ui64 cols_cnt = 0;

    auto f = [&poses, &out, &cols_cnt](std::vector<TColumnPtr> block) -> Expected<void> {
        TCsvWriter w(out);

        std::vector<i64> col_poses;

        for (ui64 j = 0; j < block.size(); j++) {
            std::vector<std::string> row(block[0]->GetSize());
            // for (ui64 i = 0; i < block[0]->GetSize(); i++) {
            //     row[i] = Do<OJfPrintIth>(block[j], i);
            // }

            col_poses.push_back(out.tellp());
            // w.WriteRow();
            auto bytes = Do<OJfPrintOpt>(block[j]);
            out.write(bytes.data(), bytes.size());
        }

        cols_cnt = block.size();

        for (auto pos : col_poses) {
            PutI64(out, pos);
        }
        poses.push_back(out.tellp());

        return nullptr;
    };

    RunCommand(f);
    
    auto meta_start = out.tellp();
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
    std::shared_ptr<std::istream> scheme,
    std::shared_ptr<std::istream> data,
    ui64 row_group_size
) {
    TEngine eng;
    auto err = eng.Setup(std::make_shared<TCsvTableInput>(scheme, data, row_group_size));
    if (!err) {
        return err.GetError();
    }
    return std::move(eng);
}

Expected<TEngine> MakeEngineFromJf(std::shared_ptr<std::istream> jf) {
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

} // namespace JfEngine
