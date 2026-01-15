#include "engine.h"

#include "utils/csvio/csv_reader.h"

#include <cassert>

#include <iostream>

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

TCSVTableInput::TCSVTableInput(
    std::istream& scheme_in,
    std::istream& data_in,
    ui64 row_group_len
) : 
    ITableInput(row_group_len),
    scheme_in_(scheme_in),
    data_in_(data_in)
{
}

IError* TCSVTableInput::GetColumnsScheme(std::vector<TRowScheme>& out) {
    while (1) {
        auto [data, err] = scheme_in_.ReadRow();

        if (err) {
            if (Is<EofErr>(err)) {
                break;
            }
            return err;
        }

        if (data.size() != 2) {
            return new IncorrrectFileErr;
        }

        out.emplace_back(data[0], data[1]);
    }

    return nullptr;
}

IError* TCSVTableInput::ReadRowGroup(std::vector<std::vector<std::string>>& out) {
    for (ui64 i = 0; i < row_group_len_; i++) {
        auto [d, err] = data_in_.ReadRow();
        if (err) {
            return err;
        }
        if (i == 0) {
            out.resize(d.size());
        } else {
            if (d.size() != out.size()) {
                return new IncorrrectFileErr("diff size");
            }
        }
        for (ui64 j = 0; j < d.size(); j++) {
            out[j].push_back(d[j]);
        }
    }

    return nullptr;
}

void TCSVTableInput::RestartDataRead() {
    data_in_.RestartRead();
}


TJFTableInput::TJFTableInput(std::istream& jf_in) : jf_in_(jf_in) {
}

i64 ReadI64(std::istream& in) {
    i64 ans = 0;
    for (ui64 i = 0; i < 8; i++) {
        ui64 c = in.get();
        ans = (ans | (c << i * 8));
    }
    return ans;
}

IError* TJFTableInput::GetColumnsScheme(std::vector<TRowScheme>& out) {
    jf_in_.seekg(0);
    auto meta_sz = ReadI64(jf_in_);

    TCSVReader scheme_in(jf_in_, meta_sz);
    while (1) {
        auto [data, err] = scheme_in.ReadRow();

        if (err) {
            if (Is<EofErr>(err)) {
                break;
            }
            return err;
        }

        if (data.size() != 2) {
            return new IncorrrectFileErr;
        }

        out.emplace_back(data[0], data[1]);
    }

    return nullptr;
}

IError* TJFTableInput::ReadRowGroup(std::vector<std::vector<std::string>>& out) {
    if (jf_in_.eof()) {
        return new EofErr;
    }
    auto meta_sz = ReadI64(jf_in_);

    TCSVReader data_in(jf_in_, meta_sz);

    for (ui64 i = 0; i < row_group_len_; i++) {
        auto [d, err] = data_in.ReadRow();

        if (err) {
            return err;
        }
        if (i == 0) {
            out.resize(d.size());
        } else {
            if (d.size() != out.size()) {
                return new IncorrrectFileErr("diff size");
            }
        }
        for (ui64 j = 0; j < d.size(); j++) {
            out[j].push_back(d[j]);
        }
    }
    return nullptr;
}

void TJFTableInput::RestartDataRead() {
    jf_in_.seekg(0);
    auto meta_sz = ReadI64(jf_in_);
    jf_in_.seekg(meta_sz);
}
