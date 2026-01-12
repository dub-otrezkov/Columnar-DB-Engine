#pragma once

#include "utils/errors/errors.h"
#include "table_node/types.h"

#include <string>
#include <fstream>
#include <vector>
#include <memory>

class TCSVWriter {
public:
    TCSVWriter(const std::string& filename, const std::string& sep);

    void WriteRow(const std::vector<std::shared_ptr<ITableNode>>& row);

    ~TCSVWriter();

private:
    std::ofstream out_;
    std::string sep_;
};
