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
    TCSVReader(const std::string& filename, const std::string& sep);

    std::pair<std::vector<std::string>, IError*> ReadRow();

private:
    std::ifstream in_;
    std::string sep_;
};
