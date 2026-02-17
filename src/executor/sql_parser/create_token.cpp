#include "tokenizer.h"

#include "engine/engine.h"
#include "ios_factory/ios_factory.h"

namespace JFEngine {

ETokens TCreateToken::GetType() const {
    return ETokens::kCreate;
}

Expected<ITableInput> TCreateToken::Exec() {
    TEngine eng;
    if (args_.size() != 1 || args_[0]->GetType() != ETokens::kNameToken) {
        return MakeError<BadCmdErr>();
    }
    auto name = std::dynamic_pointer_cast<TNameToken>(args_[0])->GetName();
    auto err = eng.Setup(TIOFactory::GetTableIO(kCurTableInput).GetShared());
    if (err.HasError()) {
        return err.GetError();
    }
    TIOFactory::RegisterFileIO(name, ETypeFile::kJFFile);
    eng.WriteTableToJF(*TIOFactory::GetIO(name).GetShared());

    return nullptr;
}

} // namespace JFEngine