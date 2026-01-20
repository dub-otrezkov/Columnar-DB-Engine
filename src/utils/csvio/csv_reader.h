#pragma once

#include "utils/errors/errors.h"

#include <string>
#include <fstream>
#include <vector>


class EofErr : public IError {
public:
    std::string Print() const override {
        return "EOF";
    }
};

class TCSVReader {
public:
    TCSVReader(std::istream& in, i64 buf_size_ = -2, char sep = ',');

    Expected<std::vector<std::string>> ReadRow();
    void RestartRead();

private:
    std::istream& in_;
    char sep_;

    i64 buf_size_;

    std::streampos init_pos_;
};
