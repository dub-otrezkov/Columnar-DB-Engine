#pragma once

#include "utils/errors/errors_templates.h"
#include "utils/faster_vectors/vector_string_2d.h"

#include <fstream>
#include <string>
#include <vector>

namespace JfEngine {

class TCsvReader {
public:
    TCsvReader(std::istream& in, char sep = ',');

    Expected<std::vector<std::string>> ReadRow();

private:
    std::istream& in_;
    char sep_;

    std::streampos init_pos_;
};

class TCsvOptimizedReader { // DO NOT USE ANYWHERE EXCEPT BIG Csv FILE READING, IT DESTROY ISTREAM CORRECTNESS
public:
    TCsvOptimizedReader(std::istream& in, char sep = ',');
    Expected<std::vector<std::string>> ReadRow();
    Expected<void> ReadRow(TVectorString2d& out); // EXPERIMENTAL

private:

    char ReadSym();
    bool EofC();
    char Peek();

    std::istream& in_;
    char sep_;

    static const ui64 kIBufSize = (1 << 21);

    char buf_[kIBufSize];
    i64 cpos_ = 0;
    ui64 av_ = 0;
};


} // namespace JfEngine
