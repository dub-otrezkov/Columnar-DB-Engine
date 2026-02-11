#include "tokenizer.h"

#include "ios_factory/ios_factory.h"

namespace JFEngine {

TNameToken::TNameToken(std::string name) :
    name_(std::move(name))
{}

std::string TNameToken::GetName() const {
    return name_;
}

ETokens TNameToken::GetType() const {
    return ETokens::ENameToken;
}

} // namespace JFEngine