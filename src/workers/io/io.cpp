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
    auto blocks_cnt = static_cast<ui64>(ReadI64(jf_in_));

    block_col_poses_.assign(blocks_cnt, std::vector<i64>(cols_cnt_));
    block_col_sizes_.assign(blocks_cnt, std::vector<ui64>(cols_cnt_));
    for (ui64 b = 0; b < blocks_cnt; b++) {
        for (ui64 j = 0; j < cols_cnt_; j++) {
            block_col_poses_[b][j] = ReadI64(jf_in_);
            block_col_sizes_[b][j] = static_cast<ui64>(ReadI64(jf_in_));
        }
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

Expected<TColumnPtr> TJfTableInput::ReadColumnFromMem(ui64 i, ui64 b) {
    i64 pos = block_col_poses_[b][i];
    i64 pos_next;
    if (static_cast<ui64>(i + 1) < cols_cnt_) {
        pos_next = block_col_poses_[b][i + 1];
    } else if (b + 1 < block_col_poses_.size()) {
        pos_next = block_col_poses_[b + 1][0];
    } else {
        pos_next = meta_start_;
    }

    jf_in_->SetPos(pos);
    ui64 len = static_cast<ui64>(pos_next - pos);

    const char* raw = nullptr;
    jf_in_->Read(raw, len);
    std::span<const char> data(raw, len);

    return MakeColumnJf(data, scheme_[i].type_);
}

Expected<TColumnPtr> TJfTableInput::Finalize(ui64 column, const std::vector<ui64>& idxs) {
    if (idxs.empty()) {
        return MakeEmptyColumn(scheme_[column].type_);
    }
    if (std::is_sorted(idxs.begin(), idxs.end()) && idxs.back() - idxs.front() + 1 == idxs.size() && idxs.front() % row_group_len_ == 0) {
        return ReadColumnFromMem(column, idxs.front() / row_group_len_);
    }
    auto ans = MakeEmptyColumn(scheme_[column].type_).GetRes();
    ans->Reserve(idxs.size());
    std::unordered_map<ui64, TColumnPtr> loaded;
    for (ui64 i = 0; i < idxs.size(); i++) {
        auto b = idxs[i] / row_group_len_;
        if (loaded.count(b) == 0) {
            loaded[b] = ReadColumnFromMem(column, b).GetRes();
        }
        ans->Append(loaded[b].get(), idxs[i] % row_group_len_);
    }
    return ans;
}

Expected<TColumnPtr> TJfTableInput::FinalizeRange(ui64 column, ui64 start, ui64 len) {
    if (len == 0) {
        return MakeEmptyColumn(scheme_[column].type_);
    }
    if (start % row_group_len_ == 0) {
        return ReadColumnFromMem(column, start / row_group_len_);
    }
    std::vector<ui64> idxs(len);
    for (ui64 i = 0; i < len; i++) {
        idxs[i] = start + i;
    }
    return Finalize(column, idxs);
}

// LAZY
Expected<TColumnPtr> TJfTableInput::ReadIthColumn(i64 i) {
    // auto t = ReadColumnFromMem(i, current_block_).GetRes();
    return Expected<TColumnPtr>(
        MakeLazyColumn(current_block_ * row_group_len_, block_col_sizes_[current_block_][i], i, scheme_[i].type_).GetRes(),
        current_block_ + 1 == block_col_poses_.size() ? EError::EofErr : EError::NoError
    );
}

Expected<TColumnPtr> TJfTableInput::ReadMinMax(i64 i) {
    const ui64 b = current_block_;
    i64 pos = block_col_poses_[b][i];
    i64 pos_next;
    if (static_cast<ui64>(i + 1) < cols_cnt_) {
        pos_next = block_col_poses_[b][i + 1];
    } else if (b + 1 < block_col_poses_.size()) {
        pos_next = block_col_poses_[b + 1][0];
    } else {
        pos_next = meta_start_;
    }

    ui64 want = std::min<ui64>(static_cast<ui64>(pos_next - pos), kColStatsTailMax);
    jf_in_->SetPos(pos_next - static_cast<i64>(want));
    const char* raw = nullptr;
    jf_in_->Read(raw, want);

    auto col = ExtractMinMax(std::span<const char>(raw, want), scheme_[i].type_);
    if (col.HasError()) {
        return col.GetError();
    }
    return Expected<TColumnPtr>(
        col.GetRes(),
        b + 1 == block_col_poses_.size() ? EError::EofErr : EError::NoError
    );
}

Expected<TColumnPtr> TJfTableInput::ReadSum(i64 i) {
    const ui64 b = current_block_;
    if (b >= block_col_poses_.size()) {
        return MakeError<EError::EofErr>();
    }
    i64 pos = block_col_poses_[b][i];
    i64 pos_next;
    if (static_cast<ui64>(i + 1) < cols_cnt_) {
        pos_next = block_col_poses_[b][i + 1];
    } else if (b + 1 < block_col_poses_.size()) {
        pos_next = block_col_poses_[b + 1][0];
    } else {
        pos_next = meta_start_;
    }

    ui64 want = std::min<ui64>(static_cast<ui64>(pos_next - pos), kColStatsTailMax);
    jf_in_->SetPos(pos_next - static_cast<i64>(want));
    const char* raw = nullptr;
    jf_in_->Read(raw, want);

    auto col = ExtractSum(std::span<const char>(raw, want), scheme_[i].type_);
    if (col.HasError()) {
        return col.GetError();
    }
    return Expected<TColumnPtr>(
        col.GetRes(),
        b + 1 == block_col_poses_.size() ? EError::EofErr : EError::NoError
    );
}

Expected<std::vector<TColumnPtr>> TJfTableInput::LoadRowGroup() {
    if (current_block_ >= block_col_poses_.size()) {
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
    current_block_++;
}

void TJfTableInput::Reset() {
    current_block_ = 0;
    current_rg_.reset();
    current_rg_err_ = EError::NoError;
}

Expected<TColumnPtr> TJfTableInput::ReadColumn(const std::string& name) {
    if (current_block_ >= block_col_poses_.size()) {
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
