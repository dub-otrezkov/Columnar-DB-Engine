#pragma once

#include "utils/errors/errors_templates.h"
#include "workers/base.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>


namespace JfEngine {

static const std::string kCurTableInput = "CURRENT_TABLE_INPUT";
static const std::string kResultScheme = "RESULT_SCHEME";
static const std::string kResultData = "RESULT_DATA";

enum ETypeFile {
    kCsvFile,
    kJfFile
};

class TIoFactory {
public:
    static std::shared_ptr<TIoFactory> Instance();

    static void RegisterSStreamIo(const std::string& alias, ETypeFile t); // FOR TESTS (and probably optimizations)

    static void UnregisterIo(const std::string& alias); // FOR TESTS (and probably optimizations)

    static void RegisterCustomIo(const std::string& alias, std::shared_ptr<std::iostream> io);

    static void RegisterFileIo(const std::string& alias, ETypeFile t);

    static void RegisterTableInput(const std::string& alias, TTableInputPtr inp);

    static std::shared_ptr<std::iostream> GetIo(const std::string& alias);

    static TTableInputPtr GetTableIo(const std::string& alias);

    static void Clear();

    static void ClearAll();

private:
    std::unordered_map<std::string, std::shared_ptr<std::iostream>> ios_;
    std::unordered_map<std::string, TTableInputPtr> iotables_;
};

} // namespace JfEngin 
