#include "tokenizer.h"

#include "engine/engine.h"

#include "utils/csvio/errors.h"

#include <optional>

namespace JFEngine {

TNameToken::TNameToken(std::string name) :
    name_(std::move(name))
{}

std::string TNameToken::GetName() const {
    return name_;
}

std::optional<TTokens> IsCommand(const std::string& cmd) {
    if (cmds.contains(cmd)) {
        return cmds.at(cmd);
    }
    return std::nullopt;
}

Tokenizer::Tokenizer(const std::string& data) {
    ss << data;
}

Expected<IToken> Tokenizer::GetNext() {
    if (ss.eof()) {
        return MakeError<EofErr>();
    }

    std::string token;

    while (1) {
        if (ss.peek() == EOF || std::isspace(ss.peek())) {
            break;
        }

        char c = ss.get();
        token += c;
    }

    while (ss.peek() != EOF && std::isspace(ss.peek())) {
        ss.get();
    }

    if (token == "FROM") {
        return std::make_shared<TFromToken>();
    } else {
        return std::make_shared<TNameToken>(token);
    }
}

Expected<TEngine> TFromToken::Exec() {
    for (const auto& t : args_) {
        if (t->GetType() != TTokens::ENameToken) {
            return MakeError<BadCmdErr>();
        }
    }

    std::unique_ptr<ITableInput> in;
    if (args_.size() == 1) {
        s.emplace_back(
            std::dynamic_pointer_cast<TNameToken>(args_[0])->GetName() + ".jf"
        );

        in = std::make_unique<TJFTableInput>(s.back());
    } else if (args_.size() == 2) {
        s.emplace_back(
            std::dynamic_pointer_cast<TNameToken>(args_[0])->GetName() + ".csv"
        );
        s.emplace_back(
            std::dynamic_pointer_cast<TNameToken>(args_[1])->GetName() + ".csv"
        );

        in = std::make_unique<TCSVTableInput>(s[s.size() - 2], s[s.size() - 1]);
    } else {
        return MakeError<BadCmdErr>();
    }

    auto res = std::make_shared<TEngine>();

    auto err = res->Setup(std::move(in));

    if (err.HasError()) {
        return err.GetError();
    }

    return res;
}

TTokens TFromToken::GetType() const {
    return TTokens::EFrom;
}

TTokens TNameToken::GetType() const {
    return TTokens::ENameToken;
}

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd) {
    std::vector<std::shared_ptr<ICommand>> st;

    Tokenizer tkz(cmd);

    while (auto cur = tkz.GetNext()) {
        switch (cur.GetShared()->GetType()) {
        case TTokens::EFrom:
            st.push_back(std::dynamic_pointer_cast<TFromToken>(cur.GetShared()));
            break;
        default:
            if (st.empty() || !st.back()) {
                return MakeError<BadCmdErr>();
            }
            st.back()->AddArg(cur.GetShared());
            break;
        }
    }

    return st;
}

TTokens TCreateToken::GetType() const {
    return TTokens::ECreate;
}


Expected<void> TCreateToken::Exec() {
    
}
} // namespace JFEngine