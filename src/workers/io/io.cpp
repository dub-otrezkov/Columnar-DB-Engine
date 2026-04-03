#include "io.h"

#include "csvio/csv_reader.h"

#include "utils/errors/errors_templates.h"

#include <cassert>
#include <unordered_map>

namespace JFEngine {

Expected<void> TCSVTableInput::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }

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
            return MakeError<EError::IncorrectFileErr>();
        }

        scheme_.emplace_back(data[0], StrToTColumn(data[1]));
    }

    return nullptr;
}

Expected<std::vector<TColumnPtr>> TCSVTableInput::LoadRowGroup() {

    // std::cout << "Ffkfkfkkfkfkfkfk" << std::endl;
    auto is_eof = false;

    std::vector<std::vector<std::string>> tmp;
    for (ui64 i = 0; i < row_group_len_; i++) {
        auto res = csv_data_.ReadRow();
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

    // if (out.size() != GetScheme().size()) {
    //     std::cout << "wtf??? " << out.size() << " " << GetScheme().size() << std::endl;
    // }

    // assert(out.size() == GetScheme().size());

    return {std::move(out), (is_eof ? MakeError<EError::EofErr>() : EError::NoError)};
}

Expected<void> TJFTableInput::SetupColumnsScheme() {
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
        current_block_ + 1 == blocks_pos_.size() ? EError::EofErr : EError::NoError
    );

    return ans;
}

Expected<std::vector<TColumnPtr>> TJFTableInput::LoadRowGroup() {
    // std::cout << "dkdkkdkfkkfdkk" << std::endl;
    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
    }

    std::vector<TColumnPtr> res;

    bool is_eof = false;

    for (ui64 i = 0; i < cols_cnt_; i++) {
        auto [col, err] = ReadIthColumn(i);

        // std::cout << "!! " << err << " " << col << std::endl;
        if (err) {
            if (!Is<EError::EofErr>(err)) {
                return err;
            } else if (err != EError::NoError) {
                is_eof = true;
            }
        }
        res.push_back(col);
    }

    // std::cout << "---" << std::endl;
    // for (auto el : res) {
    //     std::cout << " " << el;
    // }
    // std::cout << std::endl;

    assert(res.size() == GetScheme().size());

    return {std::move(res), is_eof ? MakeError<EError::EofErr>() : EError::NoError};
}

void TJFTableInput::MoveCursor(i64 delta) {
    current_rg_.reset();
    if (delta < 0 && current_block_ < -delta) {
        current_block_ = 0;
    } else {
        current_block_ += delta;
    }
    // std::cout << current_block_ << std::endl;
}

void TJFTableInput::Reset() {
    current_block_ = 0;
    current_rg_.reset();
}

Expected<IColumn> TJFTableInput::ReadColumn(const std::string& name) {
    static auto name_to_index = [this]() -> auto {
        std::unordered_map<std::string, ui64> poses;
        for (size_t i = 0; i < scheme_.size(); i++) {
            poses[scheme_[i].name_] = i;
        }
        return poses;
    };

    static auto inds = name_to_index();
    // std::cout << ":%: " << name << std::endl;
    if (name != "*" && inds.count(name) == 0) {
        return EError::NoSuchColumnsErr;
    }

    if (current_block_ >= blocks_pos_.size()) {
        return MakeError<EError::EofErr>();
    }

    if (name == "*") {
        return ReadIthColumn(0);
    }

    return ReadIthColumn(inds[name]);
}


ui64 TJFTableInput::GetGroupsCount() const {
    return blocks_pos_.size();
}


TJFNeccessaryOnly::TJFNeccessaryOnly(std::shared_ptr<std::istream> jf_in, std::string query) :
    TJFTableInput(jf_in),
    query_(std::move(query))
{
}

Expected<void> TJFNeccessaryOnly::SetupColumnsScheme() {
    auto err = TJFTableInput::SetupColumnsScheme();
    if (err.HasError()) {
        return err.GetError();
    }
    new_scheme_.clear();
    cols_.clear();
    for (ui64 i = 0; i < TJFTableInput::GetScheme().size(); i++) {
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

std::vector<TRowScheme>& TJFNeccessaryOnly::GetScheme() {
    return new_scheme_;
}

Expected<std::vector<TColumnPtr>> TJFNeccessaryOnly::LoadRowGroup() {
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

} // namespace JFEngine
