#include "engine.h"

#include "utils/csvio/csv_writer.h"

#include <sstream>
#include <cassert>
#include <iostream>

namespace JFEngine {

Expected<void> TEngine::Setup(std::unique_ptr<ITableInput>&& in) {
    in_ = std::move(in);
    return in_->GetColumnsScheme(cols_);
}

Expected<void> TEngine::WriteSchemeToCSV(std::ostream& out) {
    TCSVWriter w(out);
    for (auto col : cols_) {
        w.WriteRow({col.name_, col.type_});
    }
    return nullptr;
}

Expected<void> TEngine::ReadRowGroup(std::vector<std::shared_ptr<IColumn>>& out) {
    std::vector<std::vector<std::string>> vals;
    bool is_eof = false;
    {
        auto res = in_->ReadRowGroup(vals);
        if (res.HasError()) {
            if (Is<EofErr>(res.GetError())) {
                is_eof = true;
            } else {
                return res;
            }
        }
    }

    out.assign(vals.size(), {});

    TNodesFactory factory(cols_);

    for (ui64 col_i = 0; col_i < vals.size(); col_i++) {
        for (const auto& val : vals[col_i]) {
            auto [res, err] = factory.Make(col_i, val);
            if (err) {
                return err;
            }
            out[col_i]->push_back(res);
        }
    }

    return {is_eof ? MakeError<EofErr>() : nullptr};
}

Expected<void> TEngine::WriteDataToCSV(std::ostream& out) {
    TCSVWriter w(out);

    auto f = [&w](std::vector<std::shared_ptr<IColumn>> block) -> Expected<void> {
        for (ui64 i = 0; i < block[0]->size(); i++) {
            std::vector<std::string> row;
            for (ui64 j = 0; j < block.size(); j++) {
                row.push_back(block[j]->at(i)->Get());
            }
            w.WriteRow(row);
        }

        return nullptr;
    };

    return RunCommand(f);
}

void PutI64(std::ostream& out, i64 i) {
    for (ui64 b = 0; b < 8; b++) {
        out << char((i >> (b * 8)) & ((1 << 8) - 1));
    }
}

Expected<void> TEngine::WriteTableToJF(std::ostream& out) {

    std::vector<i64> poses;

    ui64 cols_cnt = 0;

    auto f = [&poses, &out, &cols_cnt](std::vector<std::shared_ptr<IColumn>> block) -> Expected<void> {
        TCSVWriter w(out);

        std::vector<i64> col_poses;
        
        for (ui64 i = 0; i < block[0]->size(); i++) {
            std::vector<std::string> row;
            for (ui64 j = 0; j < block.size(); j++) {
                row.push_back(block[j][i]->Get());
            }
            col_poses.push_back(out.tellp());
            w.WriteRow(row);
        }

        assert(cols_cnt == 0 || cols_cnt == col_poses.size());
        cols_cnt = col_poses.size();

        for (auto pos : col_poses) {
            PutI64(out, pos);
        }
        PutI64(out, cols_cnt);
        poses.push_back(out.tellp());

        return nullptr;
    };

    RunCommand(f);
    
    auto meta_start = out.tellp();
    PutI64(out, cols_cnt);
    PutI64(out, poses.size());
    for (auto i : poses) {
        PutI64(out, i);
    }
    auto err = WriteSchemeToCSV(out);

    PutI64(out, meta_start);

    return err;
}

Expected<TEngine> MakeEngineFromCSV(std::istream& scheme, std::istream& data) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_unique<TCSVTableInput>(scheme, data));
    if (!err) {
        std::cout << err.GetError()->Print() << std::endl;
        return err.GetError();
    }
    return eng;
}

Expected<TEngine> MakeEngineFromJF(std::istream& jf) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_unique<TJFTableInput>(jf));
    if (err) {
        return err.GetError();
    }
    return eng;
}

} // namespace JFEngine
