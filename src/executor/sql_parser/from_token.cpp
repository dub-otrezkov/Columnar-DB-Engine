#include "tokenizer.h"

#include "ios_factory/ios_factory.h"

namespace JfEngine {

Expected<TTableInputPtr> TFromToken::MakeWorker() {
    for (const auto& t : args_) {
        if (t->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>("from token without files");
        }
    }

    if (args_.size() == 1) {
        auto name = static_cast<TNameToken*>(args_[0])->GetName();

        IFileInput* io = TIoFactory::GetInput(name);
        if (!io) {
            TIoFactory::RegisterFileIo(name, ETypeFile::kJfFile);
            io = TIoFactory::GetInput(name);
        }
        return std::make_shared<TJfNeccessaryOnly>(io, referenced_columns_);
    } else if (args_.size() == 2) {
        auto scheme = static_cast<TNameToken*>(args_[0])->GetName();
        auto data = static_cast<TNameToken*>(args_[1])->GetName();

        IFileInput* scheme_io = TIoFactory::GetInput(scheme);
        if (!scheme_io) {
            TIoFactory::RegisterFileIo(scheme, ETypeFile::kCsvFile);
            scheme_io = TIoFactory::GetInput(scheme);
        }
        IFileInput* data_io = TIoFactory::GetInput(data);
        if (!data_io) {
            TIoFactory::RegisterFileIo(data, ETypeFile::kCsvFile);
            data_io = TIoFactory::GetInput(data);
        }

        return std::make_shared<TCsvTableInput>(scheme_io, data_io);
    } else {
        return MakeError<EError::BadCmdErr>("bad from command args cnt");
    }
}

} // namespace JfEngine
