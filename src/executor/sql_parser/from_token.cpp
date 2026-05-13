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

        std::shared_ptr<std::iostream> io = TIoFactory::GetIo(name);
        if (!io) {
            io = std::make_shared<std::fstream>(name + ".jf");
        }
        return std::make_shared<TJfNeccessaryOnly>(io, query_);
    } else if (args_.size() == 2) {
        auto scheme = static_cast<TNameToken*>(args_[0])->GetName();
        auto data = static_cast<TNameToken*>(args_[1])->GetName();

        std::shared_ptr<std::iostream> scheme_io = TIoFactory::GetIo(scheme);
        if (!scheme_io) {
            scheme_io = std::make_shared<std::fstream>(scheme + ".csv");
        }
        std::shared_ptr<std::iostream> data_io = TIoFactory::GetIo(data);
        if (!data_io) {
            data_io = std::make_shared<std::fstream>(data + ".csv");
        }

        return std::make_shared<TCsvTableInput>(scheme_io, data_io);
    } else {
        return MakeError<EError::BadCmdErr>("bad from command args cnt");
    }
}

} // namespace JfEngine
