#include "csv_writer.h"

#include <algorithm>


namespace JFEngine {

TCSVWriter::TCSVWriter(std::ostream& out, char sep) :
    out_(out),
    sep_(sep)
{
}

void TCSVWriter::PrepareString(std::ostream& out, std::string_view str) {
    bool ret = 1;
    ui64 cq = 0;
    for (const auto& ch : str) {
        if (ch == sep_ || ch == '\n' || ch == '\"') {
            ret = 0;
        }
        if (ch == '\"') {
            cq++;
        }
    }

    if (ret) {
        // return str;
        for (char el : str) {
            out << el;
        }
        return;
    }

    // std::string ans;
    // ans.reserve(str.size() + 2 + cq);
    // ans += '"';
    out << '"';

    for (auto ch : str) {
        if (ch == '"') {
            // ans += "\"\"";
            out << "\"\"";
        } else {
            // ans += ch;
            out << static_cast<char>(ch);
        }
    }

    // ans += '"';
    out << '"';
    // return ans;
}

void TCSVWriter::WriteRow(const std::vector<std::string>& row) {
    if (row.empty()) {
        return;
    }
    // out_ << PrepareString(row[0]);
    PrepareString(out_, row[0]);
    for (size_t i = 1; i < row.size(); i++) {
        out_ << sep_;
        PrepareString(out_, row[i]);
    }
    out_ << '\n';
}

TCSVWriter::~TCSVWriter() {
    out_.flush();
}

} // namespace JFEngine
