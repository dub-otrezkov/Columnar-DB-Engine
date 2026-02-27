#include "csv_reader.h"

#include <algorithm>
#include <iostream>

namespace JFEngine {

const ui64 kStrReserveSize = (1 << 5);

TCSVReader::TCSVReader(std::istream& in, char sep) :
    in_(in),
    sep_(sep),
    init_pos_(in_.tellg())
{
}

Expected<std::vector<std::string>> TCSVReader::ReadRow() {
    if (in_.eof()) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;

    bool in_quotes = false;

    i64 total = 0;

    std::vector<std::string> ans;
    ans.push_back("");

    while (!in_.eof()) {
        auto c = in_.get();

        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (in_.peek() == '\"') {
                    ans.back() += in_.get();
                } else if (in_.peek() != sep_ && in_.peek() != '\n' && in_.peek() != '\r') {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
            }
        } else {
            ans.back() += c;
        }
    }

    if (!read_smth) {
        return {std::move(ans), MakeError<EError::EofErr>()};
    }

    return {std::move(ans)};
}

TCSVOptimizedReader::TCSVOptimizedReader(std::istream& in, char sep) :
    in_(in),
    sep_(sep)
{
}

char TCSVOptimizedReader::ReadSym() {
    if (cpos_ < av_) {
        return buf_[cpos_++];
    }

    av_ = in_.read(buf_, kIBufSize).gcount();

    cpos_ = 0;
    return buf_[cpos_++];
}


bool TCSVOptimizedReader::EofC() {
    return cpos_ == av_ && in_.eof();
}

char TCSVOptimizedReader::Peek() {
    if (cpos_ < kIBufSize) {
        return buf_[cpos_];
    }
    return in_.peek();
}

Expected<std::vector<std::string>> TCSVOptimizedReader::ReadRow() {
    if (EofC()) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;

    bool in_quotes = false;

    Expected<std::vector<std::string>> ans_e{std::vector<std::string>()};

    auto& ans = ans_e.GetRes();
    ans.push_back("");

    while (!EofC()) {
        auto c = ReadSym();
        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (Peek() == '\"') {
                    ans.back() += ReadSym();
                } else if (Peek() != sep_ && Peek() != '\n' && Peek() != '\r') {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
            }
        } else {
            ans.back() += c;
        }
    }

    if (!read_smth) {
        return MakeError<EError::EofErr>();
    }

    return ans_e;
}

void TCSVReader::RestartRead() {
    in_.seekg(init_pos_);
}


TCSVBufferedReader::TCSVBufferedReader(std::istream& in, i64 buf_size, char sep) :
    in_(in),
    sep_(sep),
    buf_size_(buf_size),
    cur_pos_(0)
{
    buf_ = new char[buf_size_ + 1];
    buf_size_ = in_.read(buf_, buf_size_).gcount();
    buf_[buf_size_] = EOF;

}


Expected<std::vector<std::string>> TCSVBufferedReader::ReadRow() {
    if (in_.eof() || cur_pos_ == buf_size_) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;
    bool in_quotes = false;

    i64 total = 0;

    std::vector<std::string> ans;
    ans.push_back("");

    while (cur_pos_ <= buf_size_) {
        auto c = buf_[cur_pos_++];

        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (buf_[cur_pos_] == '\"') {
                    ans.back() += buf_[cur_pos_++];
                } else if (buf_[cur_pos_] != sep_ && buf_[cur_pos_] != '\n' && buf_[cur_pos_] != '\r' && buf_[cur_pos_] != EOF) {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
            }
        } else {
            ans.back() += c;
        }
    }

    if (!read_smth) {
        return {std::move(ans), MakeError<EError::EofErr>()};
    }

    return {std::move(ans)};
}

} // namespace JFEngine