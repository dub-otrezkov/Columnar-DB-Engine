#include "tokenizer.h"

#include "ios_factory/ios_factory.h"

namespace JFEngine {

Expected<ITableInput> TFromToken::Exec() {
    for (const auto& t : args_) {
        if (t->GetType() != ETokens::kNameToken) {
            return MakeError<BadCmdErr>("from token without files");
        }
    }

    if (args_.size() == 1) {
        auto name = std::dynamic_pointer_cast<TNameToken>(args_[0])->GetName();

        TIOFactory::RegisterFileIO(name, ETypeFile::kJFFile);

        return std::make_shared<TJFTableInput>(TIOFactory::GetIO(name).GetShared());
    } else if (args_.size() == 2) {
        auto scheme = std::dynamic_pointer_cast<TNameToken>(args_[0])->GetName();
        auto data = std::dynamic_pointer_cast<TNameToken>(args_[1])->GetName();

        TIOFactory::RegisterFileIO(scheme, ETypeFile::kCSVFile);
        TIOFactory::RegisterFileIO(data, ETypeFile::kCSVFile);

        return std::make_shared<TCSVTableInput>(
            TIOFactory::GetIO(scheme).GetShared(),
            TIOFactory::GetIO(data).GetShared()
        );
    } else {
        return MakeError<BadCmdErr>("bad from command args cnt");
    }
}

ETokens TFromToken::GetType() const {
    return ETokens::kFrom;
}

} // namespace JFEngine