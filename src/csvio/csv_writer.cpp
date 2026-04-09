#include "csv_writer.h"

#include <algorithm>


namespace JfEngine {

TCsvWriter::TCsvWriter(std::ostream& out, char sep) :
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
    static std::vector<ui64> lns;
    lns.assign(row.size(), 0);

    ui64 total = row.size();
    for (ui64 i = 0; i < row.size(); i++) {
        lns[i] = PrepareString(row[i]);
        total += lns[i];
    }
    static std::vector<char> tot;
    tot.resize(total);
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
    out_.write(tot.data(), total);
}


void TCsvWriter::WriteRowGroup(std::vector<std::vector<std::string>> group) {
    if (group.empty()) {
        return;
    }
    ui64 total = 0;
    static std::vector<ui64> lns;
    lns.assign(group.size() * group[0].size(), 0);
    for (ui64 i = 0; i < group.size(); i++) {
        for (ui64 j = 0; j < group[i].size(); j++) {
            lns[i * group[0].size() + j] = PrepareString(group[i][j]);
            total += 1 + lns[i * group[0].size() + j];
        }
    }

    static std::vector<char> tot;
    tot.resize(total);

    ui64 cur = 0;
    for (ui64 j = 0; j < group[0].size(); j++) {
        for (ui64 i = 0; i < group.size(); i++) {
            if (lns[i * group[0].size() + j] == group[i][j].size()) {
                memcpy(tot.data() + cur, group[i][j].data(), lns[i * group[0].size() + j]);
                cur += lns[i * group[0].size() + j];
            } else {
                tot[cur++] = '"';
                for (const auto& c : group[i][j]) {
                    tot[cur++] = c;
                    if (c == '"') {
                        tot[cur++] = c;
                    }
                }
                tot[cur++] = '"';
            }
            tot[cur++] = sep_;
        }
        tot[cur - 1] = '\n';
    }
    out_.write(tot.data(), total);
}

TCsvWriter::~TCsvWriter() {
    out_.flush();
}

} // namespace JfEngine
