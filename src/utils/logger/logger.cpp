#include "logger.h"

#include <iostream>

namespace Logger {

std::fstream& GetIO() {
    std::cout << kLogsPath << std::endl;
    static std::fstream io(kLogsPath);
    std::cout << io.good() << std::endl;
    return io;
}

}