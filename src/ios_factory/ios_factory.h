#pragma once

#include "utils/errors/errors_templates.h"
#include "utils/mmap_input/mmap_input.h"
#include "workers/base.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace JfEngine {

static const std::string kCurTableInput = "CURRENT_TABLE_INPUT";
static const std::string kResultScheme  = "RESULT_SCHEME";
static const std::string kResultData    = "RESULT_DATA";

enum ETypeFile {
    kCsvFile,
    kJfFile
};

class TIoFactory {
public:
    static std::shared_ptr<TIoFactory> Instance();

    static void RegisterSStreamIo(const std::string& alias, ETypeFile t);
    static void UnregisterIo(const std::string& alias);

    static void RegisterCustomInput(const std::string& alias, IFileInputPtr in);
    static void RegisterCustomOutput(const std::string& alias, IFileOutputPtr out);

    static void RegisterFileIo(const std::string& alias, ETypeFile t);

    static void RegisterTableInput(const std::string& alias, TTableInputPtr inp);

    static IFileInput*  GetInput(const std::string& alias);
    static IFileOutput* GetOutput(const std::string& alias);

    static TTableInputPtr GetTableIo(const std::string& alias);

    static void Clear();
    static void ClearAll();

private:
    std::unordered_map<std::string, IFileInputPtr>  inputs_;
    std::unordered_map<std::string, IFileOutputPtr> outputs_;
    std::unordered_map<std::string, TTableInputPtr> iotables_;
};

} // namespace JfEngine
