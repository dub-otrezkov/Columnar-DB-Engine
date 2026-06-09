#pragma once

#include "utils/errors/errors_templates.h"
#include "utils/mmap_input/mmap_input.h"
#include "workers/base.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace JfEngine {

static const std::string kCurTableInput = "CURRENT_TABLE_INPUT";
static const std::string kResultScheme  = "RESULT_SCHEME";
static const std::string kResultData    = "RESULT_DATA";
static const std::string kJfInput       = "JF_INPUT";

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

    static void RegisterFileInput(const std::string& alias, ETypeFile t);
    static void RegisterFileOutput(const std::string& alias, ETypeFile t);

    static void RegisterTableInput(const std::string& alias, TTableInputPtr inp);

    static TTableInputPtr RegisterJfInput(const std::string& alias, IFileInput* jf_in);
    static TTableInputPtr RegisterJfNeccessaryInput(
        const std::string& alias,
        IFileInput* jf_in,
        std::unordered_set<std::string> referenced);

    static IFileInput*  GetInput(const std::string& alias);
    static IFileOutput* GetOutput(const std::string& alias);

    static TTableInputPtr GetTableIo(const std::string& alias);

    static void Clear();

private:
    std::unordered_map<std::string, IFileInputPtr>  inputs_;
    std::unordered_map<std::string, IFileOutputPtr> outputs_;
    std::unordered_map<std::string, TTableInputPtr> iotables_;
};

}
