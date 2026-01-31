#include "executor/executor.h"

#include <iostream>

// (cd ../../build/executor/debug; make debug_exec); ../../build/executor/debug/debug_exec

int main() {
    std::cout << JFEngine::TExecutor::Execute("CREATE josh FROM scheme data").HasError() << std::endl;
}