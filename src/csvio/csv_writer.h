#pragma once

#include "columns/types/types.h"
#include "utils/errors/errors.h"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace JfEngine {

class TCsvWriter {
public:
    TCsvWriter(std::ostream& out, char sep = ',');

    void WriteRow(const std::vector<std::string>& row);
    void WriteRowGroup(std::vector<std::vector<std::string>> row);

    ~TCsvWriter();

private:
    ui64 PrepareString(std::string_view str);

    std::ostream& out_;
    char sep_;
};

} // namespace JfEngine
