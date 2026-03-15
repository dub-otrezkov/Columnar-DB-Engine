#include "tokenizer.h"

#include "workers/groupby/groupby.h"
#include "ios_factory/ios_factory.h"

namespace JFEngine {

Expected<ITableInput> TGroupToken::Exec() {
    for (const auto& t : args_) {
        if (t->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>("from token without files");
        }
    }
    TGroupByQuery cols;
    for (ui64 i = 1; i < args_.size(); i++) {
        cols.cols.push_back(static_cast<TNameToken*>(args_[i].get())->GetName());
    }

    std::cout << cols.cols.size() << std::endl;
    
    return std::make_shared<TGroupBy>(
        TIOFactory::GetTableIO(kCurTableInput).GetShared(),
        cols,
        TGlobalAgregationQuery{selects_}
    );
}

void TGroupToken::SetSelects(std::vector<std::shared_ptr<IAgregation>> s) {
    selects_ = std::move(s);
}

ETokens TGroupToken::GetType() const {
    return ETokens::kGroup;
}

} // namespace JFEngine
