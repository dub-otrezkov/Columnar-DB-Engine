#pragma once

#include "sql_parser/tokenizer.h"

#include "utils/errors/errors.h"

namespace JFEngine {

class TExecutor {
public:
    Expected<void> ExecQuery(const std::string& query);
    
};

} // namespace JFEngine