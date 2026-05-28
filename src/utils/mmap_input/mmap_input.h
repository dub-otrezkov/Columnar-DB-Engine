#pragma once

#include <utils/cint/int.h>
#include <utils/perf_stats/perf_stats.h>

#include <chrono>
#include <istream>
#include <memory>
#include <streambuf>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace JfEngine {

class IFileInput {
public:
    ~IFileInput() = default;

    virtual ui64 TellPos() const = 0;
    virtual void SetPos(i64) = 0;

    virtual void Read(char*&, ui64 n) = 0;
};

class IFileOutput {
public:
    ~IFileOutput() = default;

    virtual ui64 TellPos() const = 0;
    virtual void SetPos(i64) = 0;

    virtual void WRite(char*&, ui64 n) = 0;
};

} // namespace JfEngine
