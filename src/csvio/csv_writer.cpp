#include "csv_writer.h"

#include <algorithm>


namespace JfEngine {

TCsvWriter::TCsvWriter(IFileOutput* out, char sep) :
    out_(out),
    sep_(sep)
{
}

ui64 TCsvWriter::PrepareString(std::string_view str) {
    bool ret = 1;
    ui64 cq = 0;
    for (const auto& ch : str) {
        if (ch == sep_ || ch == '\n' || ch == '\r' || ch == '\"') {
            ret = 0;
        }
        if (ch == '\"') {
            cq++;
        }
    }

    if (ret) {
        return str.size();
    }

    return str.size() + 2 + cq;
}

void TCsvWriter::WriteRow(const std::vector<std::string>& row) {
    if (row.empty()) {
        return;
    }
    std::vector<ui64> lns(row.size(), 0);

    ui64 total = row.size();
    for (ui64 i = 0; i < row.size(); i++) {
        lns[i] = PrepareString(row[i]);
        total += lns[i];
    }
    std::vector<char> tot(total);
    ui64 cur = 0;
    for (ui64 i = 0; i < row.size(); i++) {
        if (lns[i] == row[i].size()) {
            memcpy(tot.data() + cur, row[i].data(), lns[i]);
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
    const char* p = tot.data();
    out_->Write(p, total);
}

}
