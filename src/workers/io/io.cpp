#include "io.h"

#include "csvio/csv_reader.h"

#include "utils/errors/errors_templates.h"

#include <cassert>
#include <unordered_map>

namespace JFEngine {

Expected<void> TCSVTableInput::SetupColumnsScheme() {
    scheme_.clear();

    TCSVReader csv_scheme(*scheme_in_);

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
            std::cout << "" << " " << data.size() << std::endl;
            return MakeError<EError::IncorrectFileErr>();
        }

        scheme_.emplace_back(data[0], StrToTColumn(data[1]));
    }

    return nullptr;
}

Expected<std::vector<TColumnPtr>> TCSVTableInput::ReadRowGroup() {
    auto is_eof = false;

    TCSVReader csv_data(*data_in_);

    std::vector<std::vector<std::string>> tmp;
    for (ui64 i = 0; i < row_group_len_; i++) {

        auto res = csv_data.ReadRowBufI();
        if (res.HasError()) {
            if (Is<EError::EofErr>(res.GetError())) {
                is_eof = true;
                break;
            } else {
                return res.GetError();
            }
        }
        auto d = res.GetRes();

        if (i == 0) {
            tmp.resize(d.size());
            for (auto& r : tmp) {
                r.reserve(row_group_len_);
            }
        } else {
            if (d.size() != tmp.size()) {
                return MakeError<EError::IncorrectFileErr>("diff size");
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

    return {std::move(out), (is_eof ? MakeError<EError::EofErr>() : EError::NoError)};
}

std::vector<TRowScheme>& TCSVTableInput::GetScheme() {
    return scheme_;
}

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

Expected<IColumn> TJFTableInput::ReadIthColumn(ui64 i) {
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

    Expected<IColumn> ans(
        col.GetShared(),
        current_block_ + 1 == blocks_pos_.size() ? MakeError<EError::EofErr>() : EError::NoError
    );

    return ans;
}

Expected<std::vector<TColumnPtr>> TJFTableInput::ReadRowGroup() {
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
    }

    std::vector<TColumnPtr> res;

    bool is_eof = false;

    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [col, err] = ReadIthColumn(i);
        // std::cout << col->GetSize() << " " << Is<EofErr>(err) << std::endl;
        if (!Is<EError::EofErr>(err)) {
            return err;
        } else if (err != EError::NoError) {
            is_eof = true;
        }
        res.push_back(col);
    }

    current_block_++;

    return {std::move(res), is_eof ? MakeError<EError::EofErr>() : EError::NoError};
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

void TJFTableInput::Reset() {
    current_block_ = 0;
}

Expected<IColumn> TJFTableInput::ReadColumn(const std::string& name) {
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
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
        return MakeError<EError::NoSuchColumnsErr>(name);
    }


    return ReadIthColumn(inds[name]);
}

} // namespace JFEngine
