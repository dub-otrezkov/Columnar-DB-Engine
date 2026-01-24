#include "engine/engine.h"

#include "workers/selector/selector.h"

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

        // std::cout << eng << " " << err << std::endl;

        if (err) {
            std::cout << err->Print() << std::endl;
            return 0;
        }

        std::stringstream ss;
        eng->WriteSchemeToCSV(ss);
        auto t = eng->WriteDataToCSV(ss);

        std::cout << ss.str() << std::endl;

        if (t.HasError()) {
            std::cout << t.GetError()->Print() << std::endl;
            return 0;
        }
    }
    {
        std::ifstream in("josh.jf", std::ios::binary);
        std::vector<std::string> cols{"hot", "red", "peppers"};
        // std::unorde
        auto [eng, err] = JFEngine::MakeSelectEngine(in, {cols});

        if (err) {
            std::cout << err->Print() << std::endl;
            return 0;
        }

        std::stringstream ss;
        auto e = eng->WriteSchemeToCSV(ss);
        if (e.HasError()){
            std::cout << e.GetError()->Print() << std::endl;
            return 0;
        }
        auto t = eng->WriteDataToCSV(ss);

        std::cout << ss.str() << std::endl;

        if (t.HasError()) {
            std::cout << t.GetError()->Print() << std::endl;
        }
    }

}