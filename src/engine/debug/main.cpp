#include "engine/engine.h"

#include "utils/logger/logger.h"

#include <iostream>
#include <sstream>

// (cd ../../build/engine/debug; make debug_engine); ../../build/engine/debug/debug_engine

int main() {
    {
        std::ifstream scheme("scheme.csv");
        std::ifstream data("data.csv");

        std::ofstream out("josh.jf", std::ios::binary);

        auto [eng, err] = JFEngine::MakeEngineFromCSV(scheme, data);

        eng->WriteTableToJF(out);
    }
    {
        std::ifstream in("josh.jf", std::ios::binary);
        auto [eng, err] = JFEngine::MakeEngineFromJF(in);

        std::stringstream ss;
        eng->WriteSchemeToCSV(ss);
        auto t = eng->WriteDataToCSV(ss);

        std::cout << ss.str() << std::endl;

        if (t.HasError()) {
            std::cout << t.GetError()->Print() << std::endl;
        }
    }

}