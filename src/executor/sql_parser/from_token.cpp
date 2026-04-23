#include "tokenizer.h"

#include "ios_factory/ios_factory.h"

namespace JfEngine {

Expected<ITableInput> TFromToken::MakeWorker() {
    for (const auto& t : args_) {
        if (t->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>("from token without files");
        }
    }

    if (args_.size() == 1) {
        auto name = static_cast<TNameToken*>(args_[0])->GetName();

        TIoFactory::RegisterFileIo(name, ETypeFile::kJfFile);
        return std::make_shared<TJfNeccessaryOnly>(TIoFactory::GetIo(name).GetShared(), query_);
    } else if (args_.size() == 2) {
        auto scheme = static_cast<TNameToken*>(args_[0])->GetName();
        auto data = static_cast<TNameToken*>(args_[1])->GetName();

        TIoFactory::RegisterFileIo(scheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(data, ETypeFile::kCsvFile);

        return std::make_shared<TCsvTableInput>(
            TIoFactory::GetIo(scheme).GetShared(),
            TIoFactory::GetIo(data).GetShared()
        );
    } else {
        return MakeError<EError::BadCmdErr>("bad from command args cnt");
    }
}

} // namespace JfEngine
