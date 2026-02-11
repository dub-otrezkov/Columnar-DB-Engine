#pragma once

#include "utils/errors/errors_templates.h"

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
    ECSVFile,
    EJFFile
};

class TIOFactory {
public:
    static std::shared_ptr<TIOFactory> Instance() {
        static auto factory = std::make_shared<TIOFactory>();
        return factory;
    }

    static void RegisterSStreamIO(const std::string& alias, ETypeFile t) { // FOR TESTS (and probably optimizations)
        auto i = Instance();
        i->ios_[alias] = std::make_shared<std::stringstream>(alias);
    }

    static void RegisterFileIO(const std::string& alias, ETypeFile t) {
        auto i = Instance();
        if (i->ios_.contains(alias)) {
            return;
        }
        i->ios_[alias] = std::make_shared<std::fstream>(alias + (t == EJFFile ? ".jf" : ".csv"));
    }

    static void RegisterTableInput(const std::string& alias, std::shared_ptr<ITableInput> inp) {
        auto i = Instance();
        i->iotables_[alias] = inp;
    }

    static Expected<std::iostream> GetIO(const std::string& alias) {
        auto i = Instance();
        if (i->ios_.contains(alias)) {
            return i->ios_.at(alias);
        }
        return MakeError<IONotFoundErr>(alias);
    }

    static Expected<ITableInput> GetTableIO(const std::string& alias) {
        auto i = Instance();
        if (i->iotables_.contains(alias)) {
            return i->iotables_.at(alias);
        }
        return MakeError<IONotFoundErr>(alias);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<std::iostream>> ios_;
    std::unordered_map<std::string, std::shared_ptr<ITableInput>> iotables_;
};

} // namespace JFEngin 
