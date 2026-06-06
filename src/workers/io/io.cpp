#include "io.h"

#include "csvio/csv_reader.h"
#include "utils/errors/errors_templates.h"

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <span>

namespace JfEngine {

Expected<void> TCsvTableInput::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }

    TCsvReader csv_scheme(scheme_in_);

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

    TVectorString2d tmp;

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
    }
    std::vector<TColumnPtr> out;
    for (ui64 i = 0; i < scheme_.size(); i++) {
        auto col = MakeColumnOptimized(tmp, i, scheme_[i].type_);
        if (col.HasError()) {
            return col.GetError();
        }
        out.push_back(col.GetRes());
    }

    return {std::move(out), (is_eof ? MakeError<EError::EofErr>() : EError::NoError)};
}

Expected<void> TJfTableInput::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
    jf_in_->SetPos(static_cast<i64>(jf_in_->Size()) - 8);
    meta_start_ = ReadI64(jf_in_);

    jf_in_->SetPos(meta_start_);

    row_group_len_ = ReadI64(jf_in_);
    cols_cnt_ = ReadI64(jf_in_);
    auto blocks_cnt = ReadI64(jf_in_);
    blocks_pos_.resize(blocks_cnt);
    for (ui64 i = 0; i < blocks_cnt; i++) {
        blocks_pos_[i] = ReadI64(jf_in_);
    }
    scheme_.reserve(cols_cnt_);
    TCsvReader r(jf_in_);
    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [d, err] = r.ReadRow();
        if (err != EError::NoError) {
            return err;
        }
        if (d.size() != 2) {
            return MakeError<EError::IncorrectFileErr>("bad scheme");
        }
        scheme_.emplace_back(d.at(0), StrToTColumn(d.at(1)));
    }
    return nullptr;
}

void TJfTableInput::LoadMeta() {
    if (!poses_of_cols_) {
        std::vector<ui64> p(scheme_.size());
        auto start = blocks_pos_[current_block_];
        jf_in_->SetPos(start - sizeof(ui64) * cols_cnt_ - sizeof(i64));
        const char* raw = nullptr;
        jf_in_->Read(raw, sizeof(ui64) * cols_cnt_);
        std::memcpy(p.data(), raw, sizeof(ui64) * cols_cnt_);
        column_size_ = ReadI64(jf_in_);
        p.push_back(start - sizeof(i64) * cols_cnt_ - sizeof(i64));
        poses_of_cols_ = std::move(p);
    }
}

Expected<TColumnPtr> TJfTableInput::ReadIthColumn(i64 i) {
    LoadMeta();

    bool is_eof = (current_block_ + 1 == blocks_pos_.size());

    if (!current_rg_) {
        current_rg_ = std::make_shared<std::vector<TColumnPtr>>(cols_cnt_);
    }
    if (current_rg_->at(i)) {
        return {current_rg_->at(i), is_eof ? EError::EofErr : EError::NoError};
    }

    ui64 pos = poses_of_cols_->at(i);
    ui64 pos_next = poses_of_cols_->at(i + 1);
    ui64 len = pos_next - pos;

    auto in_cp = jf_in_;
    auto tp = scheme_[i].type_;

    TColumnPtr col;

    auto pref = [in_cp, pos, len, tp, col]() mutable -> TColumnPtr {
        if (col) {
            return col;
        }
        const char* raw = nullptr;
        in_cp->SetPos(pos);
        in_cp->Read(raw, len);
        std::span<const char> data(raw, len);

        col = MakeColumnJf(data, tp).GetRes();

        return col;
    };

    auto res = MakeColumnLazy(column_size_, pref, scheme_.at(i).type_).GetRes();
    current_rg_->at(i) = res;

    return {res, is_eof ? EError::EofErr : EError::NoError};
}

Expected<TColumnPtr> TJfTableInput::ReadMinMax(i64 i) {
    LoadMeta();

    ui64 pos = poses_of_cols_->at(i);
    ui64 pos_next = poses_of_cols_->at(i + 1);
    jf_in_->SetPos(pos);
    ui64 len = pos_next - pos;

    const char* raw = nullptr;
    jf_in_->Read(raw, len);
    std::span<const char> data(raw, len);

    auto col = ExtractMinMax(data, scheme_[i].type_);

    if (col.HasError()) {
        return col.GetError();
    }

    Expected<TColumnPtr> ans(
        col.GetRes(),
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
    current_rg_err_ = EError::NoError;
    poses_of_cols_.reset();
    current_block_++;
}

void TJfTableInput::Reset() {
    current_block_ = 0;
    current_rg_.reset();
    current_rg_err_ = EError::NoError;
}

Expected<TColumnPtr> TJfTableInput::ReadColumn(const std::string& name) {
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
    }

    if (name == "*") {
        return ReadIthColumn(0);
    }

    for (size_t i = 0; i < scheme_.size(); i++) {
        if (scheme_[i].name_ == name) {
            return ReadIthColumn(i);
        }
    }
    return MakeError<EError::NoSuchColumnsErr>("no such column " + name);
}


TJfNeccessaryOnly::TJfNeccessaryOnly(IFileInput* jf_in, std::unordered_set<std::string> referenced) :
    TJfTableInput(jf_in),
    referenced_(std::move(referenced))
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
        if (referenced_.contains(scheme_[i].name_)) {
            new_scheme_.emplace_back(scheme_[i]);
            cols_.push_back(i);
        }
    }
    if (new_scheme_.empty() && !scheme_.empty()) {
        auto type_width = [](EColumn t) -> int {
            switch (t) {
                case ki8Column:        return 1;
                case ki16Column:       return 2;
                case ki32Column:       return 4;
                case kDateColumn:      return 4;
                case ki64Column:       return 8;
                case kDoubleColumn:    return 8;
                case kTimestampColumn: return 8;
                case ki128Column:      return 16;
                case kStringColumn:    return 1000;
                default:               return 1000;
            }
        };
        ui64 best = 0;
        int best_w = type_width(scheme_[0].type_);
        for (ui64 i = 1; i < scheme_.size(); i++) {
            int w = type_width(scheme_[i].type_);
            if (w < best_w) {
                best = i;
                best_w = w;
            }
        }
        new_scheme_.emplace_back(scheme_[best]);
        cols_.push_back(best);
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
