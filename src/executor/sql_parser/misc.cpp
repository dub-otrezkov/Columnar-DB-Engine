#include "tokenizer.h"

namespace JfEngine {

ETokens TOpenBracketToken::GetType() const {
    return ETokens::kOpenBracket;
}

ETokens TCloseBracketToken::GetType() const {
    return ETokens::kCloseBracket;
}

ETokens TSumToken::GetType() const {
    return ETokens::kSum;
}

ETokens TCountToken::GetType() const {
    return ETokens::kCount;
}

ETokens TAvgToken::GetType() const {
    return ETokens::kAvg;
}

ETokens TMinToken::GetType() const {
    return ETokens::kMin;
}

ETokens TMaxToken::GetType() const {
    return ETokens::kMax;
}

ETokens TComaToken::GetType() const {
    return ETokens::kComa;
}

ETokens TDistinctToken::GetType() const {
    return ETokens::kDistinct;
}

ETokens TLengthToken::GetType() const {
    return ETokens::kLength;
}

ETokens TPlusToken::GetType() const {
    return ETokens::kPlus;
}

ETokens TMinusToken::GetType() const {
    return ETokens::kMinus;
}

Expected<ITableInput> IoperatorCommand::MakeWorker() {
    return MakeError<EError::UnsupportedErr>();
}

ETokens TAsToken::GetType() const {
    return ETokens::kAs;
}

ETokens TAndToken::GetType() const {
    return ETokens::kAnd;
}

} // namespace JfEngine