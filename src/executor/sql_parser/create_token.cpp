#include "tokenizer.h"

#include "engine/engine.h"
#include "ios_factory/ios_factory.h"

namespace JFEngine {

TTokens TCreateToken::GetType() const {
    return TTokens::ECreate;
}

Expected<ITableInput> TCreateToken::Exec() {
    TEngine eng;
    if (args_.size() != 1 || args_[0]->GetType() != TTokens::ENameToken) {
        return MakeError<BadCmdErr>();
    }
    auto name = std::dynamic_pointer_cast<TNameToken>(args_[0])->GetName();
    auto err = eng.Setup(TIOFactory::GetTableIO(kCurTableInput).GetShared());
    if (err.HasError()) {
        return err.GetError();
    }
    TIOFactory::RegisterFileIO(name, TFileType::EJFFile);
    eng.WriteTableToJF(*TIOFactory::GetIO(name).GetShared());
}

} // namespace JFEngine