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

    std::shared_ptr<std::iostream> io = TIoFactory::GetIo(name);
    if (!io) {
        io = std::make_shared<std::fstream>(name + ".jf");
    }
    eng.WriteTableToJf(*io);

    return EError::NoError;
}

} // namespace JfEngine