#include "engine.h"

#include "utils/csvio/csv_reader.h"

#include <cassert>

#include <iostream>

namespace JFEngine {

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

Expected<void> TCSVTableInput::GetColumnsScheme(std::vector<TRowScheme>& out) {
    while (1) {
        auto err = scheme_in_.ReadRow();
        
        // std::cout << err.HasError() << std::endl;

        if (err.HasError()) {
            if (Is<EofErr>(err.GetError())) {
                break;
            }
            return err.GetError();
        }

        auto data = **err;

        if (data.size() != 2) {
            return MakeError<IncorrrectFileErr>();
        }

        out.emplace_back(data[0], data[1]);
    }

    return nullptr;
}

Expected<void> TCSVTableInput::ReadRowGroup(std::vector<std::vector<std::string>>& out) {
    for (ui64 i = 0; i < row_group_len_; i++) {
        auto res = data_in_.ReadRow();
        if (!res) {
            return res.GetError();
        }
        auto d = **res;
        if (i == 0) {
            out.resize(d.size());
        } else {
            if (d.size() != out.size()) {
                return MakeError<IncorrrectFileErr>("diff size");
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

Expected<void> TCSVTableInput::ReadRowGroup(std::vector<std::vector<std::string>>&, ui64) {
    return MakeError<UnimplementedErr>();
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

Expected<void> TJFTableInput::GetColumnsScheme(std::vector<TRowScheme>& out) {
    jf_in_.seekg(0);
    auto meta_sz = ReadI64(jf_in_);

    TCSVReader scheme_in(jf_in_, meta_sz);
    while (1) {
        auto res = scheme_in.ReadRow();

        if (!res) {
            if (Is<EofErr>(res.GetError())) {
                break;
            }
            return res.GetError();
        }

        auto data = **res;

        if (data.size() != 2) {
            return MakeError<IncorrrectFileErr>();
        }

        out.emplace_back(data[0], data[1]);
    }


    i64 blocks_cnt = ReadI64(jf_in_);
    
    meta_sz += 8 * (blocks_cnt + 1);
    for (ui64 i = 0; i < blocks_cnt; i++) {
        auto c = ReadI64(jf_in_);
        blocks_pos.push_back(c + meta_sz);
    }

    return nullptr;
}

Expected<void> TJFTableInput::ReadRowGroup(std::vector<std::vector<std::string>>& out) {
    if (jf_in_.eof()) {
        return MakeError<EofErr>();
    }
    auto meta_sz = ReadI64(jf_in_);

    TCSVReader data_in(jf_in_, meta_sz);

    for (ui64 i = 0; i < row_group_len_; i++) {
        auto res = data_in.ReadRow();

        if (!res) {
            return res.GetError();
        }

        auto d = **res;

        if (i == 0) {
            out.resize(d.size());
        } else {
            if (d.size() != out.size()) {
                return MakeError<IncorrrectFileErr>("diff size");
            }
        }
        for (ui64 j = 0; j < d.size(); j++) {
            out[j].push_back(d[j]);
        }
    }
    return nullptr;
}

void TJFTableInput::RestartDataRead() {
    if (blocks_pos.empty()) {
        return;
    }
    jf_in_.seekg(blocks_pos[0]);
}


Expected<void> TJFTableInput::ReadRowGroup(std::vector<std::vector<std::string>>& out, ui64 index) {
    return MakeError<UnimplementedErr>();
}

} // namespace JFEngine
