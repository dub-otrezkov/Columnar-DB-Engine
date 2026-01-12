#include "csv_reader.h"

#include <algorithm>
#include <iostream>

TCSVReader::TCSVReader(const std::string& filename, const std::string& sep) :
    sep_(sep)
{
    in_.open(filename);
    if (!in_.is_open()) {
        std::cout << "failed to open file " << filename << std::endl;
    }
}

std::pair<std::vector<std::string>, IError*> TCSVReader::ReadRow() {
    if (in_.eof()) {
        return {std::vector<std::string>(), new EofErr};
    }

    std::string tmp;

    std::vector<std::string> ans;

    ans.emplace_back();

    while (!in_.eof()) {
        auto c = in_.get();
        if (c == '\n' || c == EOF) {
            break;
        }
        ans.back() += c;
        if (ans.back().size() >= sep_.size()) {
            bool same = true;
            for (size_t i = ans.back().size() - sep_.size(); i < ans.back().size(); i++) {
                if (ans.back()[i] != sep_[i - (ans.back().size() - sep_.size())]) {
                    same = false;
                    break;
                }
            }
            if (same) {
                ans.back().resize(ans.back().size() - sep_.size());
                ans.emplace_back();
            }
        }

    }

    return {std::move(ans), nullptr};
}