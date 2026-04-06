#pragma once

#include "utils/errors/errors.h"

#include <vector>

class TVectorString2d {
public:
    inline Expected<void> At(ui64 i, ui64 j, std::string* load) const {
        ui64 k = i * width_ + j;
        if (k >= pos_.size() || width_ == 0) {
            return MakeError<EError::OutOfRangeErr>();
        }
        ui64 len;
        if (k + 1 == pos_.size()) {
            len = buf_.size() - pos_[k];
        } else {
            len = pos_[k + 1] - pos_[k];
        }
        load->resize(len);
        std::memcpy(load->data(), buf_.data() + pos_[k], len);

        return EError::NoError;
    }

    inline void NewCol() {
        pos_.push_back(buf_.size());
    }

    inline ui64 Size() const {
        if (width_ == 0) {
            return 0;
        }
        return (pos_.size() + width_ - 1) / width_;
    }

    inline void NewRow() {
        if (width_ == 0) {
            width_ = pos_.size();
        }
        NewCol();
    }

    inline void WriteSymToLastCR(ui8 sym) {
        buf_.push_back(sym);
    }

    inline bool LastEmpty() {
        return pos_.empty() || pos_.back() == buf_.size();
    }
private:
    std::vector<ui8> buf_;
    std::vector<ui64> pos_;
    mutable ui64 width_ = 0;
};
