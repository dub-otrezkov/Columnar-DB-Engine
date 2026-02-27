#include "ios_factory.h"

namespace JFEngine {

std::shared_ptr<TIOFactory> TIOFactory::Instance() {
    static auto factory = std::make_shared<TIOFactory>();
    return factory;
}

void TIOFactory::RegisterSStreamIO(const std::string& alias, ETypeFile t) { // FOR TESTS (and probably optimizations)
    auto i = Instance();
    i->ios_[alias] = std::make_shared<std::stringstream>();
}

void TIOFactory::UnregisterIO(const std::string& alias) { // FOR TESTS (and probably optimizations)
    auto i = Instance();
    i->ios_.erase(alias);
}

void TIOFactory::RegisterFileIO(const std::string& alias, ETypeFile t) {
    auto i = Instance();
    if (i->ios_.contains(alias)) {
        return;
    }
    i->ios_[alias] = std::make_shared<std::fstream>(alias + (t == kJFFile ? ".jf" : ".csv"));
}

void TIOFactory::RegisterTableInput(const std::string& alias, std::shared_ptr<ITableInput> inp) {
    auto i = Instance();
    i->iotables_[alias] = inp;
}

Expected<std::iostream> TIOFactory::GetIO(const std::string& alias) {
    auto i = Instance();
    if (i->ios_.contains(alias)) {
        return i->ios_.at(alias);
    }
    return MakeError<EError::IONotFoundErr>(alias);
}

Expected<ITableInput> TIOFactory::GetTableIO(const std::string& alias) {
    auto i = Instance();
    if (i->iotables_.contains(alias)) {
        return i->iotables_.at(alias);
    }
    return MakeError<EError::IONotFoundErr>(alias);
}

} // namespace JFEngine
