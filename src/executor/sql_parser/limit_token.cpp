#include "tokenizer.h"

#include <string>

namespace JfEngine {

Expected<TTableInputPtr> TLimitToken::MakeWorker() {
    return EError::UnsupportedErr;
}

ui64 TLimitToken::GetLimit() const {
    if (args_.size() != 1) {
        return 0;
    }
    if (args_[0]->GetType() != ETokens::kNameToken) {
        return 0;
    }

    auto str = static_cast<TNameToken*>(args_[0])->GetName();

    try {
        auto ans = std::stoll(str);

        return ans;
    } catch (...) {
        return 0;
    }
}


Expected<TTableInputPtr> TOffsetToken::MakeWorker() {
    return EError::UnsupportedErr;
}

ui64 TOffsetToken::GetOffset() const {
    if (args_.size() != 1) {
        return 0;
    }
    if (args_[0]->GetType() != ETokens::kNameToken) {
        return 0;
    }

    auto str = static_cast<TNameToken*>(args_[0])->GetName();

    try {
        auto ans = std::stoll(str);

        return ans;
    } catch (...) {
        return 0;
    }
}

} // namespace JfEngine
