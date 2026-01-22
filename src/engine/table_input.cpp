#include "engine.h"
#include "errors.h"

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
{}

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
            return MakeError<IncorrectFileErr>();
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
                return MakeError<IncorrectFileErr>("diff size");
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

TJFTableInput::TJFTableInput(std::istream& jf_in) : jf_in_(jf_in) {
}

Expected<void> TJFTableInput::GetColumnsScheme() {
    jf_in_.seekg(-8, std::ios::end);
    meta_start_ = ReadI64(jf_in_);

    jf_in_.seekg(meta_start_, std::ios::beg);

    row_group_len_ = ReadI64(jf_in_);
    cols_cnt_ = ReadI64(jf_in_);
    auto blocks_cnt = ReadI64(jf_in_);
    blocks_pos_.resize(blocks_cnt);
    for (ui64 i = 0; i < blocks_cnt; i++) {
        blocks_pos_[i] = ReadI64(jf_in_);
    }
    scheme_.reserve(cols_cnt_);
    TCSVReader r(jf_in_);
    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [d, err] = r.ReadRow();
        if (err) {
            return err;
        }
        if (d->size() != 2) {
            return MakeError<IncorrectFileErr>("bad scheme");
        }
        scheme_.emplace_back(d->at(0), d->at(1));
    }
}

Expected<std::vector<std::shared_ptr<IColumn>>> TJFTableInput::ReadRowGroup() {
    
    if (current_block_ == blocks_pos_.size()) {
        return MakeError<EofErr>();
    }

    auto start = blocks_pos_[current_block_];
    // current_block_ = (current_block_ + 1) % blocks_pos_.size();
    current_block_++;


    TCSVReader rr(jf_in_);

    std::vector<std::shared_ptr<IColumn>> res;

    for (ui64 i = 0; i < cols_cnt_; i++) {
        jf_in_.seekg(start - 8 * (cols_cnt_ - i));
        auto pos = ReadI64(jf_in_);
        jf_in_.seekg(pos);

        auto d = rr.ReadRow();
        if (d.HasError()) {
            return d.GetError();
        }
        // std::cout << scheme_[i].type_ << std::endl;
        auto col = MakeColumnJF(d.GetRes(), scheme_[i].type_);

        if (col.HasError()) {
            return col.GetError();
        }

        res.push_back(col.GetShared());
    }

    return res;
}

void TJFTableInput::RestartDataRead() {
}

std::vector<TRowScheme>& TJFTableInput::GetScheme() {
    return scheme_;
}

} // namespace JFEngine
