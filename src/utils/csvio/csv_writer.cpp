#include "csv_writer.h"

#include <algorithm>

TCSVWriter::TCSVWriter(std::ostream& out, char sep) :
    out_(out),
    sep_(sep)
{
}

std::string TCSVWriter::PrepareString(const std::string& str) {
    if (std::count(str.begin(), str.end(), sep_) == 0 && std::count(str.begin(), str.end(), '"') == 0) {
        return str;
    }

    std::string ans;
    ans += '"';

    for (auto ch : str) {
        if (ch == '"') {
            ans += "\"\"";
        } else {
            ans += ch;
        }
    }

    ans += '"';
    return ans;
}

void TCSVWriter::WriteRow(std::shared_ptr<JFEngine::IColumn> row) {
    // if (row.empty()) {
    //     return;
    // }
    // out_ << PrepareString(row[0]->Get());
    // for (size_t i = 1; i < row.size(); i++) {
    //     out_ << sep_ << PrepareString(row[i]->Get());
    // }
    // out_ << '\n';
}

void TCSVWriter::WriteRow(const std::vector<std::string>& row) {
    if (row.empty()) {
        return;
    }
    out_ << PrepareString(row[0]);
    for (size_t i = 1; i < row.size(); i++) {
        out_ << sep_ << PrepareString(row[i]);
    }
    out_ << '\n';
}

TCSVWriter::~TCSVWriter() {
    out_.flush();
}
