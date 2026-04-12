#include "tokenizer.h"

#include "workers/orderby/orderby.h"
#include "ios_factory/ios_factory.h"

namespace JfEngine {

Expected<ITableInput> TOrderToken::MakeWorker() {
    for (const auto& t : args_) {
        if (t->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>("from token without files");
        }
    }
    TOrderByQuery query;
    for (ui64 i = 1; i < args_.size(); i++) {
        auto str = static_cast<TNameToken*>(args_[i].get())->GetName();
        if (str == "ASC") {
            query.reverse = false;
        } else if (str == "DESC") {
            query.reverse = true;
        } else {
            query.cols.push_back(str);
        }
    }

    if (limit_) {
        query.limit = limit_->GetLimit();
    }
    
    return std::make_shared<TOrderBy>(
        TIoFactory::GetTableIo(kCurTableInput).GetShared(),
        std::move(query)
    );
}

} // namespace JfEngine
