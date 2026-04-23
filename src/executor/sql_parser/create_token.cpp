#include "tokenizer.h"

#include "engine/engine.h"
#include "ios_factory/ios_factory.h"

namespace JfEngine {

Expected<TTableInputPtr> TCreateToken::MakeWorker() {
    TEngine eng;
    if (args_.size() != 1 || args_[0]->GetType() != ETokens::kNameToken) {
        return MakeError<EError::BadCmdErr>();
    }
    auto name = static_cast<TNameToken*>(args_[0])->GetName();
    auto err = eng.Setup(TIoFactory::GetTableIo(kCurTableInput));
    if (err.HasError()) {
        return err.GetError();
    }
    TIoFactory::RegisterFileIo(name, ETypeFile::kJfFile);

    eng.WriteTableToJf(*TIoFactory::GetIo(name));

    return EError::NoError;
}

} // namespace JfEngine