#include "csv_writer.h"

#include <algorithm>


namespace JFEngine {

TCSVWriter::TCSVWriter(std::ostream& out, char sep) :
    out_(out),
    sep_(sep)
{
}

ui64 TCSVWriter::PrepareString(std::string_view str) {
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
        return str.size();
    }

    return str.size() + 2 + cq;


    // std::string ans;
    // ans.reserve(str.size() + 2 + cq);
    // ans += '"';
    // out << '"';

    // for (auto ch : str) {
    //     if (ch == '"') {
    //         // ans += "\"\"";
    //         out << "\"\"";
    //     } else {
    //         // ans += ch;
    //         out << static_cast<char>(ch);
    //     }
    // }

    // // ans += '"';
    // out << '"';
    // return ans;
}

void TCSVWriter::WriteRow(const std::vector<std::string>& row) {
    if (row.empty()) {
        return;
    }

    ui64 total = row.size();
    std::vector<ui64> lns(row.size());
    for (ui64 i = 0; i < row.size(); i++) {
        lns[i] = PrepareString(row[i]);
        total += lns[i];
    }
    auto tot = new char[total];
    ui64 cur = 0;
    for (ui64 i = 0; i < row.size(); i++) {
        if (lns[i] == row[i].size()) {
            memcpy(tot + cur, row[i].data(), lns[i]);
            cur += lns[i];
        } else {
            tot[cur++] = '"';
            for (const auto& c : row[i]) {
                tot[cur++] = c;
                if (c == '"') {
                    tot[cur++] = c;
                }
            }
            tot[cur++] = '"';
        }

        if (i + 1 != row.size()) {
            tot[cur++] = sep_;
        }
    }
    tot[cur] = '\n';
    out_.write(tot, total);
    // out_ << PrepareString(row[0]);
    // PrepareString(out_, row[0]);
    // for (size_t i = 1; i < row.size(); i++) {
    //     out_ << sep_;
    //     PrepareString(out_, row[i]);
    // }
    // out_ << '\n';

}

TCSVWriter::~TCSVWriter() {
    out_.flush();
}

} // namespace JFEngine
