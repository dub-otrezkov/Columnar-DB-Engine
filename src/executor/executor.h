#pragma once

#include "sql_parser/tokenizer.h"
#include "utils/errors/errors.h"
#include "utils/perf_stats/perf_stats.h"

#include <iostream>

namespace JfEngine {

class TExecutor {
public:
    Expected<void> ExecQuery(const std::string& query);

    ~TExecutor();

private:
    TQueryStats stats_;
};

} // namespace JfEngine 