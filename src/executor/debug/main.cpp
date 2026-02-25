#include "executor/executor.h"

#include "ios_factory/ios_factory.h"

#include <iostream>

// (cd ../../build/executor/debug; make debug_exec); ../../build/executor/debug/debug_exec

int main() {
    JFEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, dorothy");
        std::cout << err.HasError() << std::endl;
        if (err.HasError()) {
        }
    }
    {
        auto err = exec.ExecQuery("SELECT SUM(WatchID) FROM josh");
        std::cout << err.HasError() << std::endl;
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
    }
}