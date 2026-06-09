#include "ios_factory.h"

#include "workers/io/io.h"

#include <fstream>
#include <sstream>

namespace JfEngine {

std::shared_ptr<TIoFactory> TIoFactory::Instance() {
    static auto factory = std::make_shared<TIoFactory>();
    return factory;
}

void TIoFactory::RegisterSStreamIo(const std::string& alias, ETypeFile) {
    auto i = Instance();
    auto ss = std::make_shared<std::stringstream>();
    i->inputs_[alias]  = std::make_shared<TIstreamFileInput>(ss);
    i->outputs_[alias] = std::make_shared<TOstreamFileOutput>(ss);
}

void TIoFactory::UnregisterIo(const std::string& alias) {
    auto i = Instance();
    i->inputs_.erase(alias);
    i->outputs_.erase(alias);
}

void TIoFactory::RegisterCustomInput(const std::string& alias, IFileInputPtr in) {
    auto i = Instance();
    i->inputs_[alias] = std::move(in);
}

void TIoFactory::RegisterCustomOutput(const std::string& alias, IFileOutputPtr out) {
    auto i = Instance();
    i->outputs_[alias] = std::move(out);
}

void TIoFactory::RegisterFileInput(const std::string& alias, ETypeFile t) {
    auto i = Instance();
    if (i->inputs_.contains(alias)) {
        return;
    }
    auto path = alias + (t == kJfFile ? ".jf" : ".csv");
    i->inputs_[alias] = std::make_shared<TMMapFileInput>(path);
}

void TIoFactory::RegisterFileOutput(const std::string& alias, ETypeFile t) {
    auto i = Instance();
    if (i->outputs_.contains(alias)) {
        return;
    }
    auto path = alias + (t == kJfFile ? ".jf" : ".csv");
    i->outputs_[alias] = std::make_shared<TOstreamFileOutput>(
        std::make_shared<std::ofstream>(path)
    );
}

void TIoFactory::RegisterTableInput(const std::string& alias, TTableInputPtr inp) {
    auto i = Instance();
    i->iotables_[alias] = inp;
}

TTableInputPtr TIoFactory::RegisterJfInput(const std::string& alias, IFileInput* jf_in) {
    auto i = Instance();
    auto inp = std::make_shared<TJfTableInput>(jf_in);
    i->iotables_[alias] = inp;
    return inp;
}

TTableInputPtr TIoFactory::RegisterJfNeccessaryInput(
    const std::string& alias,
    IFileInput* jf_in,
    std::unordered_set<std::string> referenced
) {
    auto i = Instance();
    auto inp = std::make_shared<TJfNeccessaryOnly>(jf_in, std::move(referenced));
    i->iotables_[alias] = inp;
    return inp;
}

IFileInput* TIoFactory::GetInput(const std::string& alias) {
    auto i = Instance();
    if (auto it = i->inputs_.find(alias); it != i->inputs_.end()) {
        return it->second.get();
    }
    return nullptr;
}

IFileOutput* TIoFactory::GetOutput(const std::string& alias) {
    auto i = Instance();
    if (auto it = i->outputs_.find(alias); it != i->outputs_.end()) {
        return it->second.get();
    }
    return nullptr;
}

TTableInputPtr TIoFactory::GetTableIo(const std::string& alias) {
    auto i = Instance();
    if (auto it = i->iotables_.find(alias); it != i->iotables_.end()) {
        return it->second;
    }
    return nullptr;
}

void TIoFactory::Clear() {
    auto i = Instance();
    i->iotables_.clear();
}

// Defined here (not in nodes_lib) so the column layer can trigger late
// materialization through TIoFactory without depending on it directly.
// Declared in columns/types/types.h.
TColumnPtr LazyMaterialize(ui64 column, const std::vector<ui64>& idxs) {
    auto in = TIoFactory::GetTableIo(kJfInput);
    auto* jf = static_cast<TJfTableInput*>(in.get());
    return jf->Finalize(column, idxs).GetRes();
}

}
