#pragma once

#include "utils/errors/errors_templates.h"

#include "workers/base.h"

#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace JFEngine {

static const std::string kCurTableInput = "CURRENT_TABLE_INPUT";
static const std::string kResultScheme = "RESULT_SCHEME";
static const std::string kResultData = "RESULT_DATA";

enum ETypeFile {
    kCSVFile,
    kJFFile
};

class TIOFactory {
public:
    static std::shared_ptr<TIOFactory> Instance();

    static void RegisterSStreamIO(const std::string& alias, ETypeFile t); // FOR TESTS (and probably optimizations)

    static void UnregisterIO(const std::string& alias); // FOR TESTS (and probably optimizations)

    static void RegisterFileIO(const std::string& alias, ETypeFile t);

    static void RegisterTableInput(const std::string& alias, std::shared_ptr<ITableInput> inp);

    static Expected<std::iostream> GetIO(const std::string& alias);

    static Expected<ITableInput> GetTableIO(const std::string& alias);

private:
    std::unordered_map<std::string, std::shared_ptr<std::iostream>> ios_;
    std::unordered_map<std::string, std::shared_ptr<ITableInput>> iotables_;
};

} // namespace JFEngin 
