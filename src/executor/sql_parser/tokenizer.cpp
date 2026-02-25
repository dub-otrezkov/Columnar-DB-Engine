#include "tokenizer.h"

#include "engine/engine.h"
#include "ios_factory/ios_factory.h"

#include "utils/errors/errors_templates.h"

#include <optional>

namespace JFEngine {

std::optional<ETokens> IsCommand(const std::string& cmd) {
    if (cmds.contains(cmd)) {
        return cmds.at(cmd);
    }
    return std::nullopt;
}

TTokenizer::TTokenizer(const std::string& data) {
    ss << data;
}

Expected<IToken> TTokenizer::GetNext() {
    if (ss.eof()) {
        return MakeError<EError::EofErr>();
    }

    std::string token;

    while (1) {
        if (token.size() == 0 && (ss.peek() == '(' || ss.peek() == ')' || ss.peek() == ',')) {
            token += ss.get();
            break;
        }
        if (ss.peek() == EOF || std::isspace(ss.peek()) || ss.peek() == '(' || ss.peek() == ')' || ss.peek() == ',') {
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
    } else if (token == "SELECT") {
        return std::make_shared<TSelectToken>();
    } else if (token == "SUM") {
        return std::make_shared<TSumToken>();
    } else if (token == "COUNT") {
        return std::make_shared<TCountToken>();
    } else if (token == "AVG") {
        return std::make_shared<TAvgToken>();
    } else if (token == "(") {
        return std::make_shared<TOpenBracketToken>();
    } else if (token == ")") {
        return std::make_shared<TCloseBracketToken>();
    } else if (token == ",") {
        return std::make_shared<TComaToken>();
    } else {
        return std::make_shared<TNameToken>(token);
    }
}

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd) {
    std::vector<std::shared_ptr<ICommand>> st;

    TTokenizer tkz(cmd);

    while (auto cur = tkz.GetNext()) {
        switch (cur.GetShared()->GetType()) {
        case ETokens::kOpenBracket:
        case ETokens::kComa:
            break;
        case ETokens::kFrom:
            st.push_back(std::dynamic_pointer_cast<TFromToken>(cur.GetShared()));
            break;
        case ETokens::kCreate:
            st.push_back(std::dynamic_pointer_cast<TCreateToken>(cur.GetShared()));
            break;
        case ETokens::kSelect:
            st.push_back(std::dynamic_pointer_cast<TSelectToken>(cur.GetShared()));
            break;
        case ETokens::kCloseBracket:
        default: {
            if (st.empty() || !st.back()) {
                return MakeError<EError::BadCmdErr>("failed parse query");
            }
            st.back()->AddArg(cur.GetShared());
            break;
        }
        }
    }

    return st;
}

} // namespace JFEngine