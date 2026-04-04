#include "tokenizer.h"

#include <string>

namespace JfEngine {

Expected<ITableInput> TLimitToken::MakeWorker() {
    return EError::UnsupportedErr;
}

ui64 TLimitToken::GetLimit() {
    if (args_.size() != 1) {
        std::cout << "wtf???" << " " << args_.size() << std::endl;
        return 0;
    }
    if (args_[0]->GetType() != ETokens::kNameToken) {
        std::cout << "wtf???" << std::endl;
        return 0;
    }

    auto str = static_cast<TNameToken*>(args_[0].get())->GetName();

    try {
        auto ans = std::stoll(str);

        return ans;
    } catch (...) {
        return 0;
    }
}

ETokens TLimitToken::GetType() const {
    return ETokens::kLimit;
}

} // namespace JfEngine
