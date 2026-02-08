#pragma once

#include "utils/errors/errors_templates.h"

#include <string>
#include <fstream>
#include <vector>

namespace JFEngine {

constexpr i64 kUnlimitedBuffer = -2;

class TCSVReader {
public:
    TCSVReader(std::istream& in, i64 buf_size_ = kUnlimitedBuffer, char sep = ',');

    Expected<std::vector<std::string>> ReadRow();
    void RestartRead();

private:
    std::istream& in_;
    char sep_;

    i64 buf_size_;

    std::streampos init_pos_;
};

} // namespace JFEngine
