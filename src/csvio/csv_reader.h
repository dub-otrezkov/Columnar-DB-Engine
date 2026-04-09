#pragma once

#include "utils/errors/errors_templates.h"

#include "utils/faster_vectors/vector_string_2d.h"

#include <string>
#include <fstream>
#include <vector>

namespace JfEngine {

class TCsvReader {
public:
    TCsvReader(std::istream& in, char sep = ',');

    Expected<std::vector<std::string>> ReadRow();
    void RestartRead();

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

    static const ui64 kIBufSize = (1 << 20);

    char buf_[kIBufSize];
    i64 cpos_ = 0;
    ui64 av_ = 0;
};

class TCsvBufferedReader {
public:
    TCsvBufferedReader(std::istream& in, i64 buf_size = 0, char sep = ',');

    Expected<std::vector<std::string>> ReadRow();

private:
    std::istream& in_;
    char sep_;

    i64 buf_size_;
    ui64 cur_pos_ = 0;

    char* buf_;
};

} // namespace JfEngine
