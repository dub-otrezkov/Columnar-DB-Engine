#include "logger.h"

#include <iostream>

namespace Logger {

std::fstream& GetIo() {
    std::cout << kLogsPath << std::endl;
    static std::fstream io(kLogsPath);
    std::cout << io.good() << std::endl;
    return io;
}

}