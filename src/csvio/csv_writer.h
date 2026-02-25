#pragma once

#include "utils/errors/errors.h"
#include "columns/types/types.h"

#include <string>
#include <fstream>
#include <vector>
#include <memory>

namespace JFEngine {

class TCSVWriter {
public:
    TCSVWriter(std::ostream& out, char sep = ',');

    void WriteRow(const std::vector<std::string>& row);

    ~TCSVWriter();

private:
    ui64 PrepareString(std::string_view str);

    std::ostream& out_;
    char sep_;
};

} // namespace JFEngine
