#include "executor/executor.h"

#include <iostream>

// (cd ../../build/executor/debug; make debug_exec); ../../build/executor/debug/debug_exec

int main() {
    JFEngine::TExecutor exec;
    auto err = exec.ExecQuery("CREATE josh FROM scheme data");
    if (err.HasError()) {
        std::cout << err.GetError()->Print() << std::endl;
    }
}