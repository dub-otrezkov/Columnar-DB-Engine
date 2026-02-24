#include "executor/executor.h"

#include "ios_factory/ios_factory.h"

#include <iostream>

// (cd ../../build/executor/debug; make debug_exec); ../../build/executor/debug/debug_exec

int main() {
    // std::cout << " jfjfj" << std::endl;
    JFEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError()->Print() << std::endl;
        }
    }
    // {
    //     auto err = exec.ExecQuery("SELECT hot, red, peppers FROM josh");
    //     if (err.HasError()) {
    //         std::cout << err.GetError()->Print() << std::endl;
    //     }
    // }
}