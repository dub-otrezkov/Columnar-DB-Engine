#include "tokenizer.h"

namespace JFEngine {

ETokens TOpenBracketToken::GetType() const {
    return ETokens::kOpenBracket;
}

ETokens TCloseBracketToken::GetType() const {
    return ETokens::kCloseBracket;
}

ETokens TSumToken::GetType() const {
    return ETokens::kSum;
}

<<<<<<< HEAD
ETokens TCountToken::GetType() const {
    return ETokens::kCount;
}

ETokens TAvgToken::GetType() const {
    return ETokens::kAvg;
}

=======
>>>>>>> main
ETokens TComaToken::GetType() const {
    return ETokens::kComa;
}

Expected<ITableInput> IOperatorCommand::Exec() {
    return MakeError<UnsupportedErr>();
}

} // namespace JFEngine