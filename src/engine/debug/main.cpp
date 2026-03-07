#include "engine/engine.h"

#include "workers/selector/selector.h"

#include "utils/logger/logger.h"

#include <iostream>
#include <sstream>
#include <fstream>

// (cd ../../build/engine/debug; make debug_engine); ../../build/engine/debug/debug_engine

using namespace JFEngine;

int main() {
    auto scheme = std::make_shared<std::fstream>("scheme.csv");
    auto data = std::make_shared<std::fstream>("data.csv");
    auto [eng, err] = MakeEngineFromCSV(scheme, data);

}