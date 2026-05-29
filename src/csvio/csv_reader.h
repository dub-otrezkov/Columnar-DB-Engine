#pragma once

#include "utils/errors/errors_templates.h"
#include "utils/faster_vectors/vector_string_2d.h"
#include "utils/mmap_input/mmap_input.h"

#include <string>
#include <vector>

namespace JfEngine {

class TCsvReader {
public:
    TCsvReader(IFileInput* in, char sep = ',');

    Expected<std::vector<std::string>> ReadRow();

private:
    IFileInput* in_;
    char sep_;
};

class TCsvOptimizedReader {
public:
    TCsvOptimizedReader(IFileInput* in, char sep = ',');
    Expected<std::vector<std::string>> ReadRow();
    Expected<void> ReadRow(TVectorString2d& out);

private:

    int ReadSym();
    bool EofC();
    int Peek();
    void Refill();

    IFileInput* in_ = nullptr;
    char sep_;

    static const ui64 kIBufSize = (1 << 21);

    const char* buf_ = nullptr;
    i64 cpos_ = 0;
    ui64 av_ = 0;
};


} // namespace JfEngine
