#include "engine/engine.h"

#include <iostream>

// (cd ../../build/engine/debug; make debug_engine); ../../build/engine/debug/debug_engine

int main() {
    std::ifstream scheme("scheme.csv");
    std::ifstream data("data.csv");

    std::ofstream out("josh.jf");

    auto [eng, err] = JFEngine::MakeEngineFromCSV(scheme, data);

    eng->WriteTableToJF(out);
}