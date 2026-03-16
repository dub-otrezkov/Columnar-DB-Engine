#include "executor/executor.h"

#include "ios_factory/ios_factory.h"

#include <iostream>

// (cd ../../build/executor/debug; make debug_exec)
// >josh.jf; >tmp1.jf; >tmp2.jf; >RESULT_DATA.csv; >RESULT_SCHEME.csv; ../../build/executor/debug/debug_exec

int main() {
    JFEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM ssch, data");
        std::cout << err.HasError() << std::endl;
        if (err.HasError()) {
        }
    }
    {
        auto err = exec.ExecQuery("SELECT author, COUNT(*) AS c, SUM(author) FROM josh GROUP BY author HAVING c > 1 ORDER c");
        std::cout << err.HasError() << std::endl;
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
    }
}