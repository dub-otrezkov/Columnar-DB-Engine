#include "io.h"
#include "errors.h"

#include "utils/csvio/csv_reader.h"

#include <cassert>
#include <unordered_map>

namespace JFEngine {

Expected<void> TCSVTableInput::SetupColumnsScheme() {
    scheme_.clear();

    TCSVReader csv_scheme(*scheme_in_);

    while (1) {
        auto err = csv_scheme.ReadRow();

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

Expected<std::vector<TColumnPtr>> TCSVTableInput::ReadRowGroup() {
    auto is_eof = false;

    TCSVReader csv_data(*data_in_);

    std::vector<std::vector<std::string>> tmp;
    for (ui64 i = 0; i < row_group_len_; i++) {
        auto res = csv_data.ReadRow();
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
    std::vector<TColumnPtr> out;
    for (ui64 i = 0; i < tmp.size(); i++) {
        auto col = MakeColumn(std::move(tmp[i]), scheme_[i].type_);
        if (col.HasError()) {
            return col.GetError();
        }
        out.push_back(col.GetShared());
    }
    return {std::move(out), (is_eof ? MakeError<EofErr>() : nullptr)};
}

std::vector<TRowScheme>& TCSVTableInput::GetScheme() {
    return scheme_;
}

// Expected<void> TCSVTableInput::ReadRowGroup(std::vector<std::vector<std::string>>&, ui64) {
//     return MakeError<UnimplementedErr>();
// }

// TJFTableInput::TJFTableInput(std::istream&& jf_in) : jf_in_(std::move(jf_in)) {
// }

Expected<void> TJFTableInput::SetupColumnsScheme() {
    jf_in_->seekg(-8, std::ios::end);
    meta_start_ = ReadI64(*jf_in_);

    jf_in_->seekg(meta_start_, std::ios::beg);

    row_group_len_ = ReadI64(*jf_in_);
    cols_cnt_ = ReadI64(*jf_in_);
    auto blocks_cnt = ReadI64(*jf_in_);
    blocks_pos_.resize(blocks_cnt);
    for (ui64 i = 0; i < blocks_cnt; i++) {
        blocks_pos_[i] = ReadI64(*jf_in_);
    }
    scheme_.reserve(cols_cnt_);
    TCSVReader r(*jf_in_);
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
    return nullptr;
}

Expected<TColumnPtr> TJFTableInput::ReadIthColumn(ui64 i) {
    auto start = blocks_pos_[current_block_];
    jf_in_->seekg(start - sizeof(i64) * (cols_cnt_ - i));
    auto pos = ReadI64(*jf_in_);
    jf_in_->seekg(pos);

    TCSVReader rr(*jf_in_);
    auto d = rr.ReadRow();

    if (d.HasError()) {
        return d.GetError();
    }

    auto col = MakeColumnJF(d.GetRes(), scheme_[i].type_);

    if (col.HasError()) {
        return col.GetError();
    }

    return col.GetShared();
}

Expected<std::vector<TColumnPtr>> TJFTableInput::ReadRowGroup() {
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EofErr>();
    }


    std::vector<TColumnPtr> res;

    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [col, err] = ReadIthColumn(i);
        if (err) {
            return err;
        }
        res.push_back(*col);
    }
    
    current_block_++;

    return res;
}

std::vector<TRowScheme>& TJFTableInput::GetScheme() {
    return scheme_;
}

void TJFTableInput::MoveCursor(i64 delta) {
    if (delta < 0 && current_block_ < -delta) {
        current_block_ = 0;
    } else {
        current_block_ += delta;
    }
}

Expected<TColumnPtr> TJFTableInput::ReadColumn(const std::string& name) {
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EofErr>();
    }

    static auto name_to_index = [this]() -> auto {
        std::unordered_map<std::string, ui64> poses;
        for (size_t i = 0; i < scheme_.size(); i++) {
            poses[scheme_[i].name_] = i;
        }
        return poses;
    };

    static auto inds = name_to_index();

    if (inds.count(name) == 0) {
        return MakeError<NoSuchColumnsErr>(name);
    }

    return ReadIthColumn(inds[name]);
}

} // namespace JFEngine
