#include "csv_writer.h"

TCSVWriter::TCSVWriter(const std::string& filename, const std::string& sep) :
    out_(filename),
    sep_(sep)
{
}

void TCSVWriter::WriteRow(const std::vector<std::shared_ptr<ITableNode>>& row) {
    if (row.empty()) {
        return;
    }
    out_ << row[0]->Get();
    for (size_t i = 1; i < row.size(); i++) {
        out_ << sep_ << row[i];
    }
    out_ << '\n';
}

TCSVWriter::~TCSVWriter() {
    out_.flush();
}