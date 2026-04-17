#include "ios_factory.h"

namespace JfEngine {

std::shared_ptr<TIoFactory> TIoFactory::Instance() {
    static auto factory = std::make_shared<TIoFactory>();
    return factory;
}

void TIoFactory::RegisterSStreamIo(const std::string& alias, ETypeFile t) { // FOR TESTS (and probably optimizations)
    auto i = Instance();
    i->ios_[alias] = std::make_shared<std::stringstream>();
}

void TIoFactory::UnregisterIo(const std::string& alias) { // FOR TESTS (and probably optimizations)
    auto i = Instance();
    i->ios_.erase(alias);
}

void TIoFactory::RegisterCustomIo(const std::string& alias, std::shared_ptr<std::iostream> io) {
    auto i = Instance();
    i->ios_[alias] = std::move(io);
}

void TIoFactory::RegisterFileIo(const std::string& alias, ETypeFile t) {
    auto i = Instance();
    if (i->ios_.contains(alias)) {
        return;
    }
    i->ios_[alias] = std::make_shared<std::fstream>(alias + (t == kJfFile ? ".jf" : ".csv"));
}

void TIoFactory::RegisterTableInput(const std::string& alias, std::shared_ptr<ITableInput> inp) {
    auto i = Instance();
    i->iotables_[alias] = inp;
}

Expected<std::iostream> TIoFactory::GetIo(const std::string& alias) {
    auto i = Instance();
    if (i->ios_.contains(alias)) {
        return i->ios_.at(alias);
    }
    return MakeError<EError::IoNotFoundErr>(alias);
}

Expected<ITableInput> TIoFactory::GetTableIo(const std::string& alias) {
    auto i = Instance();
    if (i->iotables_.contains(alias)) {
        return i->iotables_.at(alias);
    }
    return MakeError<EError::IoNotFoundErr>(alias);
}

void TIoFactory::Clear() {
    auto i = Instance();
    i->iotables_.clear();
}

} // namespace JfEngine
