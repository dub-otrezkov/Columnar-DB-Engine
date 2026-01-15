#include "csv_reader.h"

#include <algorithm>
#include <iostream>

TCSVReader::TCSVReader(std::istream& in, i64 buf_size, char sep) :
    in_(in),
    sep_(sep),
    init_pos_(in_.tellg()),
    buf_size_(buf_size)
{
}

std::pair<std::vector<std::string>, IError*> TCSVReader::ReadRow() {
    // std::cout << buf_size_ << std::endl;
    if (in_.eof() || buf_size_ == 0) {
        return {std::vector<std::string>(), new EofErr};
    }

    // std::cout << buf_size_ << std::endl;

    std::vector<std::string> ans;

    ans.emplace_back();

    bool read_smth = false;

    bool in_quotes = false;

    while (!in_.eof() && (buf_size_ == -2 || buf_size_ > 0)) {
        auto c = in_.get();

        if (buf_size_ > 0) {
            buf_size_--;
        }

        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), new EofErr};
        }
        if ((!in_quotes && c == '\n') || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (in_.peek() == '"') {
                    ans.back() += in_.get();
                    if (buf_size_ > 0) {
                        buf_size_--;
                    }
                } else if (in_.peek() != sep_ && in_.peek() != '\n') {
                    return {std::vector<std::string>(), new EofErr};
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
        return {std::move(ans), new EofErr};
    }

    return {std::move(ans), nullptr};
}

void TCSVReader::RestartRead() {
    in_.seekg(init_pos_);
}