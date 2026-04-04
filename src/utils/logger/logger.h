#pragma once

#include <fstream>
#include <string>

namespace Logger {

const std::string kLogsPath = "~/coursework/Columnar-DB-Engine/logs/logs.txt";

std::fstream& GetIo();

template<typename T>
inline void Print(const T& obj) {
    GetIo() << "[" << __FILE__ << ": " << __LINE__ << " ]: " << obj << std::endl;
}

template<typename T>
inline void Error(const T& obj) {
    GetIo() << "ERROR [" << __FILE__ << ": " << __LINE__ << " ]: " << obj << std::endl;
}

} // namespace Logger
