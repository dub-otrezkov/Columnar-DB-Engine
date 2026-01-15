#include "engine.h"

#include "utils/csvio/csv_writer.h"

#include <sstream>
#include <cassert>
#include <iostream>

IError* TEngine::Setup(std::unique_ptr<ITableInput>&& in) {
    in_ = std::move(in);
    return in_->GetColumnsScheme(cols_);
}

IError* TEngine::WriteSchemeToCSV(std::ostream& out) {
    TCSVWriter w(out);
    for (auto col : cols_) {
        w.WriteRow({col.name_, col.type_});
    }
    return nullptr;
}

IError* TEngine::ReadRowGroup(std::vector<std::vector<std::shared_ptr<ITableNode>>>& out) {
    std::vector<std::vector<std::string>> vals;
    bool is_eof = false;
    {
        auto err = in_->ReadRowGroup(vals);
        if (err) {
            if (Is<EofErr>(err)) {
                is_eof = true;
            } else {
                return err;
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
            out[col_i].push_back(res);
        }
    }

    return (is_eof ? new EofErr : nullptr);
}

IError* TEngine::WriteDataToCSV(std::ostream& out) {
    TCSVWriter w(out);

    auto f = [&w](std::vector<std::vector<std::shared_ptr<ITableNode>>> block) -> IError* {
        for (ui64 i = 0; i < block[0].size(); i++) {
            std::vector<std::string> row;
            for (ui64 j = 0; j < block.size(); j++) {
                row.push_back(block[j][i]->Get());
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

IError* TEngine::WriteTableToJF(std::ostream& out) {
    std::stringstream scheme_str;
    auto err = WriteSchemeToCSV(scheme_str);

    auto s = scheme_str.str();

    PutI64(out, s.size());
    out << s;

    auto f = [&out](std::vector<std::vector<std::shared_ptr<ITableNode>>> block) -> IError* {
        std::stringstream ss;
        TCSVWriter w(ss);
        for (ui64 i = 0; i < block[0].size(); i++) {
            std::vector<std::string> row;
            for (ui64 j = 0; j < block.size(); j++) {
                row.push_back(block[j][i]->Get());
            }
            w.WriteRow(row);
        }

        auto s = ss.str();

        PutI64(out, s.size());
        out << s;

        return nullptr;
    };

    RunCommand(f);

    return nullptr;
}

std::pair<std::shared_ptr<TEngine>, IError*> MakeEngineFromCSV(std::istream& scheme, std::istream& data) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_unique<TCSVTableInput>(scheme, data));
    if (err) {
        // std::cout << err->Print() << std::endl;
        return {nullptr, err};
    }
    return {eng, nullptr};
}

std::pair<std::shared_ptr<TEngine>, IError*> MakeEngineFromJF(std::istream& jf) {
    auto eng = std::make_shared<TEngine>();
    auto err = eng->Setup(std::make_unique<TJFTableInput>(jf));
    if (err) {
        std::cout << err->Print() << std::endl;
        return {nullptr, err};
    }
    return {eng, nullptr};
}
