#include "io.h"

#include "csvio/csv_reader.h"

#include "utils/errors/errors_templates.h"

#include <cassert>
#include <unordered_map>

namespace JfEngine {

Expected<void> TCsvTableInput::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }

    TCsvReader csv_scheme(*scheme_in_);

    while (1) {
        auto err = csv_scheme.ReadRow();

        if (err.HasError()) {
            if (Is<EError::EofErr>(err.GetError())) {
                break;
            }
            return err.GetError();
        }

        auto data = err.GetRes();

        if (data.size() != 2) {
            return MakeError<EError::IncorrectFileErr>();
        }

        scheme_.emplace_back(data[0], StrToTColumn(data[1]));
    }

    return nullptr;
}

Expected<std::vector<TColumnPtr>> TCsvTableInput::LoadRowGroup() {

    auto is_eof = false;

    static TVectorString2d tmp;

    tmp.Clear();

    // std::vector<std::vector<std::string>> tmp;
    for (ui64 i = 0; i < row_group_len_; i++) {
        auto res = csv_data_.ReadRow(tmp);
        if (res.HasError()) {
            if (Is<EError::EofErr>(res.GetError())) {
                is_eof = true;
                break;
            } else {
                return res.GetError();
            }
        }
        // auto d = res.GetRes();

        // if (i == 0) {
        //     tmp.resize(d.size());
        //     for (auto& r : tmp) {
        //         r.reserve(row_group_len_);
        //     }
        // } else {
        //     if (d.size() != tmp.size()) {
        //         return MakeError<EError::IncorrectFileErr>("diff size");
        //     }
        // }
        // for (ui64 j = 0; j < d.size(); j++) {
        //     tmp[j].push_back(d[j]);
        // }
    }
    std::vector<TColumnPtr> out;
    for (ui64 i = 0; i < scheme_.size(); i++) {
        auto col = MakeColumnOptimized(tmp, i, scheme_[i].type_);
        if (col.HasError()) {
            return col.GetError();
        }
        out.push_back(col.GetShared());
    }

    return {std::move(out), (is_eof ? MakeError<EError::EofErr>() : EError::NoError)};
}

Expected<void> TJfTableInput::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
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
    TCsvReader r(*jf_in_);
    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [d, err] = r.ReadRow();
        if (err != EError::NoError) {
            return err;
        }
        if (d->size() != 2) {
            return MakeError<EError::IncorrectFileErr>("bad scheme");
        }
        scheme_.emplace_back(d->at(0), StrToTColumn(d->at(1)));
    }
    return nullptr;
}

Expected<IColumn> TJfTableInput::ReadIthColumn(ui64 i) {
    auto start = blocks_pos_[current_block_];
    jf_in_->seekg(start - sizeof(i64) * (cols_cnt_ - i));
    auto pos = ReadI64(*jf_in_);
    ui64 pos_next;
    if (i + 1 == scheme_.size()) {
        pos_next = start - sizeof(i64) * (cols_cnt_ - 0);
    } else {
        pos_next = ReadI64(*jf_in_);
    }
    jf_in_->seekg(pos);

    // TCsvReader rr(*jf_in_);
    ui64 len = pos_next - pos;

    // auto d = rr.ReadRow();

    // if (d.HasError() && d.GetError() != EError::EofErr) {
    //     return d.GetError();
    // }

    // auto col = MakeColumnJf(d.GetRes(), scheme_[i].type_);
    std::vector<char> data(len);
    jf_in_->read(data.data(), len);

    auto col = MakeColumnJf(data, scheme_[i].type_);

    if (col.HasError()) {
        return col.GetError();
    }

    Expected<IColumn> ans(
        col.GetShared(),
        current_block_ + 1 == blocks_pos_.size() ? EError::EofErr : EError::NoError
    );

    return ans;
}

Expected<std::vector<TColumnPtr>> TJfTableInput::LoadRowGroup() {
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
    }

    std::vector<TColumnPtr> res;

    bool is_eof = false;

    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [col, err] = ReadIthColumn(i);

        if (err) {
            if (!Is<EError::EofErr>(err)) {
                return err;
            } else if (err != EError::NoError) {
                is_eof = true;
            }
        }
        res.push_back(col);
    }

    assert(res.size() == GetScheme().size());

    return {std::move(res), is_eof ? MakeError<EError::EofErr>() : EError::NoError};
}

void TJfTableInput::MoveCursor() {
    current_rg_.reset();
    // if (delta < 0 && current_block_ < -delta) {
    //     current_block_ = 0;
    // } else {
    //     current_block_ += delta;
    // }
    current_block_++;
}

void TJfTableInput::Reset() {
    current_block_ = 0;
    current_rg_.reset();
}

Expected<IColumn> TJfTableInput::ReadColumn(const std::string& name) {
    static auto name_to_index = [this]() -> auto {
        std::unordered_map<std::string, ui64> poses;
        for (size_t i = 0; i < scheme_.size(); i++) {
            poses[scheme_[i].name_] = i;
        }
        return poses;
    };

    static auto inds = name_to_index();
    if (name != "*" && inds.count(name) == 0) {
        return MakeError<EError::NoSuchColumnsErr>("no such column " + name);
    }

    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
    }

    if (name == "*") {
        return ReadIthColumn(0);
    }

    return ReadIthColumn(inds[name]);
}


TJfNeccessaryOnly::TJfNeccessaryOnly(std::shared_ptr<std::istream> jf_in, std::string query) :
    TJfTableInput(jf_in),
    query_(std::move(query))
{
}

Expected<void> TJfNeccessaryOnly::SetupColumnsScheme() {
    auto err = TJfTableInput::SetupColumnsScheme();
    if (err.HasError()) {
        return err.GetError();
    }
    new_scheme_.clear();
    cols_.clear();
    for (ui64 i = 0; i < TJfTableInput::GetScheme().size(); i++) {
        if (query_.contains(scheme_[i].name_)) {
            new_scheme_.emplace_back(scheme_[i]);
            cols_.push_back(i);
        }
    }
    if (new_scheme_.empty()) {
        new_scheme_.emplace_back(scheme_[0]);
        cols_.push_back(0);
    }
    return EError::NoError;
}

std::vector<TRowScheme>& TJfNeccessaryOnly::GetScheme() {
    return new_scheme_;
}

Expected<std::vector<TColumnPtr>> TJfNeccessaryOnly::LoadRowGroup() {
    bool is_eof = false;
    std::vector<TColumnPtr> res;
    for (auto i : cols_) {
        auto [r, err] = ReadIthColumn(i);
        if (err) {
            if (err == EError::EofErr) {
                is_eof = true;
            } else {
                return err;
            }
        }
        res.push_back(r);
    }
    assert(res.size() == GetScheme().size());
    return {std::move(res), is_eof ? EError::EofErr : EError::NoError};
}

} // namespace JfEngine
