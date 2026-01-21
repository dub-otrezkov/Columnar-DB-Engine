#include "engine.h"

#include "utils/csvio/csv_reader.h"

#include <cassert>

#include <iostream>

namespace JFEngine {

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

ui64 ITableInput::GetRowGroupLen() const {
    return row_group_len_;
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

Expected<void> TCSVTableInput::GetColumnsScheme() {
    scheme_.clear();
    while (1) {
        auto err = scheme_in_.ReadRow();

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

        scheme_.emplace_back(data[0], data[1]);
    }

    return nullptr;
}

Expected<std::vector<std::shared_ptr<IColumn>>> TCSVTableInput::ReadRowGroup() {
    auto is_eof = false;

    std::vector<std::vector<std::string>> tmp;
    for (ui64 i = 0; i < row_group_len_; i++) {
        auto res = data_in_.ReadRow();
        if (!res) {
            if (Is<EofErr>(res.GetError())) {
                is_eof = true;
                break;
            } else {
                return res.GetError();
            }
        }
        auto d = res.GetRes();

        if (i == 0) {
            tmp.resize(d.size());
        } else {
            if (d.size() != tmp.size()) {
                return MakeError<IncorrrectFileErr>("diff size");
            }
        }

        for (ui64 j = 0; j < d.size(); j++) {
            tmp[j].push_back(d[j]);
        }
    }
    std::vector<std::shared_ptr<IColumn>> out;
    for (ui64 i = 0; i < tmp.size(); i++) {
        auto col = MakeColumn(std::move(tmp[i]), scheme_[i].type_);
        if (col.HasError()) {
            return col.GetError();
        }
        out.push_back(col.GetShared());
    }
    return {std::move(out), (is_eof ? MakeError<EofErr>() : nullptr)};
}

void TCSVTableInput::RestartDataRead() {
    data_in_.RestartRead();
}

std::vector<TRowScheme>& TCSVTableInput::GetScheme() {
    return scheme_;
}

// Expected<void> TCSVTableInput::ReadRowGroup(std::vector<std::vector<std::string>>&, ui64) {
//     return MakeError<UnimplementedErr>();
// }

// NEED TO FIX!!!

TJFTableInput::TJFTableInput(std::istream& jf_in) : jf_in_(jf_in) {
}

Expected<void> TJFTableInput::GetColumnsScheme() {
}

Expected<std::vector<std::shared_ptr<IColumn>>> TJFTableInput::ReadRowGroup() {
}

void TJFTableInput::RestartDataRead() {
}

std::vector<TRowScheme>& TJFTableInput::GetScheme() {
    return scheme_;
}

} // namespace JFEngine
