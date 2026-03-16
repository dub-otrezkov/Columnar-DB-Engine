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

    if (ss.peek() == '\'') {
        ss.get();
        while (!ss.eof() && ss.peek() != '\'') {
            token += ss.get();
            if (ss.eof()) {
                return MakeError<EError::EofErr>();
            }
        }
        if (ss.eof()) {
            return MakeError<EError::EofErr>();
        }
        ss.get();
        while (ss.peek() != EOF && std::isspace(ss.peek())) {
            ss.get();
        }
        return std::make_shared<TNameToken>(token);
    }

    while (1) {
        if (token.empty() && (ss.peek() == '(' || ss.peek() == ')' || ss.peek() == ',')) {
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
    } else if (token == "LIMIT") {
        return std::make_shared<TLimitToken>();
    } else if (token == "ORDER") {
        return std::make_shared<TOrderToken>();
    } else if (token == "(") {
        return std::make_shared<TOpenBracketToken>();
    } else if (token == ")") {
        return std::make_shared<TCloseBracketToken>();
    } else if (token == ",") {
        return std::make_shared<TComaToken>();
    } else if (token == "WHERE" || token == "HAVING") {
        return std::make_shared<TWhereToken>();
    } else if (token == "GROUP") {
        return std::make_shared<TGroupToken>();
    } else if (token == "AS") {
        return std::make_shared<TAsToken>();
    } else {
        return std::make_shared<TNameToken>(token);
    }
}

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd) {
    std::vector<std::shared_ptr<ICommand>> st;
    std::vector<std::shared_ptr<ICommand>> ags_need;

    TTokenizer tkz(cmd);

    while (auto cur = tkz.GetNext()) {
        switch (cur.GetShared()->GetType()) {
        case ETokens::kOpenBracket:
        case ETokens::kComa:
            break;
        case ETokens::kFrom:
            st.push_back(std::dynamic_pointer_cast<TFromToken>(cur.GetShared()));
            ags_need.push_back(st.back());
            break;
        case ETokens::kCreate:
            st.push_back(std::dynamic_pointer_cast<TCreateToken>(cur.GetShared()));
            ags_need.push_back(st.back());
            break;
        case ETokens::kSelect:
            st.push_back(std::dynamic_pointer_cast<TSelectToken>(cur.GetShared()));
            ags_need.push_back(st.back());
            break;
        case ETokens::kWhere:
            st.push_back(std::dynamic_pointer_cast<TWhereToken>(cur.GetShared()));
            ags_need.push_back(st.back());
            break;
        case ETokens::kGroup:
            st.push_back(std::dynamic_pointer_cast<TGroupToken>(cur.GetShared()));
            ags_need.push_back(st.back());
            break;
        case ETokens::kOrder:
            st.push_back(std::dynamic_pointer_cast<TOrderToken>(cur.GetShared()));
            ags_need.push_back(st.back());
            break;
        case ETokens::kLimit:
            // st.push_back(std::dynamic_pointer_cast<TOrderToken>(cur.GetShared()));
            if (!st.empty()) {
                if (st.back()->GetType() == ETokens::kOrder) {
                    static_cast<TOrderToken*>(st.back().get())->limit_ = std::dynamic_pointer_cast<TLimitToken>(cur.GetShared());
                } else if (st.back()->GetType() == ETokens::kGroup) {
                    static_cast<TGroupToken*>(st.back().get())->limit_ = std::dynamic_pointer_cast<TLimitToken>(cur.GetShared());
                } else {
                    st.push_back(std::dynamic_pointer_cast<TLimitToken>(cur.GetShared()));
                }
            }
            ags_need.push_back(std::dynamic_pointer_cast<TLimitToken>(cur.GetShared()));
            break;
        case ETokens::kCloseBracket:
        default: {
            if (st.empty() || !st.back()) {
                return MakeError<EError::BadCmdErr>("failed parse query");
            }
            ags_need.back()->AddArg(cur.GetShared());
            break;
        }
        }
    }

    return st;
}

} // namespace JFEngine