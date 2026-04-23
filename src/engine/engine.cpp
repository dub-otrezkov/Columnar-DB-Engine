#include "engine.h"

#include "columns/operators/operators.h"
#include "csvio/csv_writer.h"

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

    auto f = [&w](std::vector<TColumnPtr> block) -> Expected<void> {
        for (ui64 i = 0; i < block[0]->GetSize(); i++) {
            std::vector<std::string> row(block.size());
            for (ui64 j = 0; j < block.size(); j++) {
                row[j] = Do<OPrintIth>(block[j], i);
            }
            w.WriteRow(row);
        }

        return nullptr;
    };

    return RunCommand(f);
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
