#pragma once

#include "columns/types/types.h"
#include "utils/errors/errors.h"
#include "utils/mmap_input/mmap_input.h"

#include <string>
#include <string_view>
#include <vector>

namespace JfEngine {

class TCsvWriter {
public:
    TCsvWriter(IFileOutput* out, char sep = ',');
    ~TCsvWriter() = default;

    void WriteRow(const std::vector<std::string>& row);

private:
    ui64 PrepareString(std::string_view str);

    IFileOutput* out_;
    char sep_;
};

}
