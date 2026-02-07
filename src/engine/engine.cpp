#include "engine.h"

#include "utils/csvio/csv_writer.h"
#include "table_node/operators.h"

#include <sstream>
#include <cassert>
#include <iostream>

namespace JFEngine {

Expected<void> TEngine::Setup(std::shared_ptr<ITableInput> in) {
    in_ = std::move(in);
    return in_->SetupColumnsScheme();
}

Expected<void> TEngine::WriteSchemeToCSV(std::ostream& out) {
    TCSVWriter w(out);
    for (auto col : in_->GetScheme()) {
        w.WriteRow({col.name_, col.type_});
    }
    return nullptr;
}

Expected<void> TEngine::WriteDataToCSV(std::ostream& out) {
    TCSVWriter w(out);

    auto f = [&w](std::vector<TColumnPtr> block) -> Expected<void> {
        for (ui64 i = 0; i < block[0]->GetSize(); i++) {
            std::vector<std::string> row;
            for (ui64 j = 0; j < block.size(); j++) {
                row.push_back(Do<OPrintIth>(block[j], i));
            }
            w.WriteRow(row);
        }

        return nullptr;
    };

    return RunCommand(f);
}

Expected<void> TEngine::WriteTableToJF(std::ostream& out) {
    std::vector<i64> poses;

    ui64 cols_cnt = 0;

    auto f = [&poses, &out, &cols_cnt](std::vector<TColumnPtr> block) -> Expected<void> {
        TCSVWriter w(out);

        std::vector<i64> col_poses;

        for (ui64 j = 0; j < block.size(); j++) {
            std::vector<std::string> row;
            for (ui64 i = 0; i < block[0]->GetSize(); i++) {
                row.push_back(Do<OJFPrintIth>(block[j], i));
            }
            col_poses.push_back(out.tellp());
            w.WriteRow(row);
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
    auto err = WriteSchemeToCSV(out);

    PutI64(out, meta_start);

    return err;
}

Expected<TEngine> MakeEngineFromCSV(
    std::shared_ptr<std::istream> scheme,
    std::shared_ptr<std::istream> data,
    ui64 row_group_size
) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_shared<TCSVTableInput>(scheme, data, row_group_size));
    if (!err) {
        return err.GetError();
    }
    return eng;
}

Expected<TEngine> MakeEngineFromJF(std::shared_ptr<std::istream> jf) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_shared<TJFTableInput>(jf));
    if (err.HasError()) {
        return err.GetError();
    }
    return eng;
}

Expected<TEngine> MakeSelectEngine(
    std::shared_ptr<std::istream> jf,
    TSelectQuery query
) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_shared<TSelector>(std::make_shared<TJFTableInput>(jf), query));
    if (err.HasError()) {
        return err.GetError();
    }
    return eng;
}

Expected<TEngine> MakeEngineFromWorker(std::shared_ptr<ITableInput>&& worker) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(worker);
    if (err.HasError()) {
        return err.GetError();
    }
    return eng;
}

} // namespace JFEngine
