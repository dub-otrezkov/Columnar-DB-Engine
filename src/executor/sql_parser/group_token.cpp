#include "tokenizer.h"

#include "ios_factory/ios_factory.h"

namespace JFEngine {

Expected<ITableInput> TGroupToken::Exec() {
    
}

void TGroupToken::SetSelects(std::vector<std::shared_ptr<IAgregation>> s) {
    selects_ = std::move(s);
}

ETokens TGroupToken::GetType() const {
    return ETokens::kFrom;
}

} // namespace JFEngine
