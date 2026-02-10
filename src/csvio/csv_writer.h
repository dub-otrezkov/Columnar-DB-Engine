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
    std::string PrepareString(const std::string& str);

    std::ostream& out_;
    char sep_;
};

} // namespace JFEngine
