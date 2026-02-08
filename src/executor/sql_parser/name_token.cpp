#include "tokenizer.h"

#include "ios_factory/ios_factory.h"

namespace JFEngine {

TNameToken::TNameToken(std::string name) :
    name_(std::move(name))
{}

std::string TNameToken::GetName() const {
    return name_;
}

TTokens TNameToken::GetType() const {
    return TTokens::ENameToken;
}

} // namespace JFEngine