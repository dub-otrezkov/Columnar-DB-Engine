#include "tokenizer.h"

#include "engine/engine.h"
#include "ios_factory/ios_factory.h"

#include "utils/csvio/errors.h"

#include <optional>

namespace JFEngine {

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
    } else if (token == "CREATE") {
        return std::make_shared<TCreateToken>();
    } else {
        return std::make_shared<TNameToken>(token);
    }
}

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd) {
    std::vector<std::shared_ptr<ICommand>> st;

    Tokenizer tkz(cmd);

    while (auto cur = tkz.GetNext()) {
        switch (cur.GetShared()->GetType()) {
        case TTokens::EFrom:
            st.push_back(std::dynamic_pointer_cast<TFromToken>(cur.GetShared()));
            break;
        case TTokens::ECreate:
            st.push_back(std::dynamic_pointer_cast<TCreateToken>(cur.GetShared()));
            break;
        default:
            if (st.empty() || !st.back()) {
                return MakeError<BadCmdErr>("failed parse query");
            }
            st.back()->AddArg(cur.GetShared());
            break;
        }
    }

    return st;
}

} // namespace JFEngine