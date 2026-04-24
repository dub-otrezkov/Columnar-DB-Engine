#include "tokenizer.h"

#include "engine/engine.h"
#include "ios_factory/ios_factory.h"
#include "utils/errors/errors_templates.h"
#include "workers/global_agregations/agregator.h"

#include <optional>

namespace JfEngine {

std::optional<ETokens> IsCommand(const std::string& cmd) {
    if (cmds.contains(cmd)) {
        return cmds.at(cmd);
    }
    return std::nullopt;
}

TTokenizer::TTokenizer(const std::string& data) : query_(data) {
    ss << data;
}

Expected<std::unique_ptr<IToken>> TTokenizer::GetNext() {
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
        return std::make_unique<TNameToken>(token);
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
        return std::make_unique<TFromToken>(query_);
    } else if (token == "CREATE") {
        return std::make_unique<TCreateToken>();
    } else if (token == "SELECT") {
        return std::make_unique<TSelectToken>();
    } else if (token == "SUM") {
        return std::make_unique<TSumToken>();
    } else if (token == "COUNT") {
        return std::make_unique<TCountToken>();
    } else if (token == "AVG") {
        return std::make_unique<TAvgToken>();
    } else if (token == "MIN") {
        return std::make_unique<TMinToken>();
    } else if (token == "MAX") {
        return std::make_unique<TMaxToken>();
    } else if (token == "DISTINCT") {
        return std::make_unique<TDistinctToken>();
    } else if (token == "LENGTH") {
        return std::make_unique<TLengthToken>();
    } else if (token == "EXTRACT_MINUTE") {
        return std::make_unique<TExtractMinuteToken>();
    } else if (token == "TRUNC_MINUTE") {
        return std::make_unique<TTruncMinuteToken>();
    } else if (token == "LIMIT") {
        return std::make_unique<TLimitToken>();
    } else if (token == "CONST_INT") {
        return std::make_unique<TConstIntToken>();
    } else if (token == "IF") {
        return std::make_unique<TIfToken>();
    } else if (token == "COND") {
        return std::make_unique<TCondToken>();
    } else if (token == "THEN") {
        return std::make_unique<TThenToken>();
    } else if (token == "ELSE") {
        return std::make_unique<TElseToken>();
    } else if (token == "OFFSET") {
        return std::make_unique<TOffsetToken>();
    } else if (token == "+") {
        return std::make_unique<TPlusToken>();
    } else if (token == "-") {
        return std::make_unique<TMinusToken>();
    } else if (token == "ORDER") {
        return std::make_unique<TOrderToken>();
    } else if (token == "(") {
        return std::make_unique<TOpenBracketToken>();
    } else if (token == ")") {
        return std::make_unique<TCloseBracketToken>();
    } else if (token == ",") {
        return std::make_unique<TComaToken>();
    } else if (token == "WHERE" || token == "HAVING") {
        return std::make_unique<TWhereToken>();
    } else if (token == "GROUP") {
        return std::make_unique<TGroupToken>();
    } else if (token == "AS") {
        return std::make_unique<TAsToken>();
    } else if (token == "AND") {
        return std::make_unique<TAndToken>();
    } else {
        return std::make_unique<TNameToken>(token);
    }
}

Expected<std::vector<std::unique_ptr<IOa>>> ParseIf(std::vector<IToken*>& tkz) {
    std::vector<std::unique_ptr<IOa>> ans;
    std::vector<IToken*> cond;
    for (ui64 i = 0; i < tkz.size(); i++) {
        auto raw = tkz[i];
        if (raw->GetType() == ETokens::kCloseBracket) {
            break;
        } else if (raw->GetType() == ETokens::kThen) {
            ans.push_back(
                std::make_unique<TColumnOp>(
                    static_cast<TNameToken*>(tkz[i + 1])->GetName()
                )
            );
        } else if (raw->GetType() == ETokens::kElse) {
            ans.push_back(
                std::make_unique<TColumnOp>(
                    static_cast<TNameToken*>(tkz[i + 1])->GetName()
                )
            );
        } else {
            cond.push_back(raw);
        }
    }
    auto [fils, err] = ParseWhereConfig(cond);
    if (err) {
        return err;
    }
    auto ift = std::make_unique<TIfOp>();
    ift->cond = std::move(TFilterQuery{fils});
    ans.push_back(std::move(ift));
    return std::move(ans);
}

Expected<TParsedCommand> ParseCommand(const std::string& cmd) {
    std::vector<std::unique_ptr<IToken>> all;
    std::vector<ICommand*> st;
    std::vector<ICommand*> ags_need;

    TTokenizer tkz(cmd);

    while (auto cur = tkz.GetNext()) {
        auto token_up = std::move(cur.GetRes());
        IToken* raw = token_up.get();

        switch (raw->GetType()) {
        case ETokens::kOpenBracket:
        case ETokens::kComa:
            all.push_back(std::move(token_up));
            break;
        case ETokens::kFrom: {
            auto* p = static_cast<TFromToken*>(raw);
            all.push_back(std::move(token_up));
            st.push_back(p);
            ags_need.push_back(p);
            break;
        }
        case ETokens::kCreate: {
            auto* p = static_cast<TCreateToken*>(raw);
            all.push_back(std::move(token_up));
            st.push_back(p);
            ags_need.push_back(p);
            break;
        }
        case ETokens::kSelect: {
            auto* p = static_cast<TSelectToken*>(raw);
            all.push_back(std::move(token_up));
            st.push_back(p);
            ags_need.push_back(p);
            break;
        }
        case ETokens::kWhere: {
            auto* p = static_cast<TWhereToken*>(raw);
            all.push_back(std::move(token_up));
            st.push_back(p);
            ags_need.push_back(p);
            break;
        }
        case ETokens::kGroup: {
            auto* p = static_cast<TGroupToken*>(raw);
            all.push_back(std::move(token_up));
            st.push_back(p);
            ags_need.push_back(p);
            break;
        }
        case ETokens::kOrder: {
            auto* p = static_cast<TOrderToken*>(raw);
            all.push_back(std::move(token_up));
            st.push_back(p);
            ags_need.push_back(p);
            break;
        }
        case ETokens::kLimit: {
            auto* p = static_cast<TLimitToken*>(raw);
            all.push_back(std::move(token_up));
            if (!st.empty()) {
                if (st.back()->GetType() == ETokens::kOrder) {
                    static_cast<TOrderToken*>(st.back())->limit_ = p;
                } else if (st.back()->GetType() == ETokens::kGroup) {
                    static_cast<TGroupToken*>(st.back())->limit_ = p;
                } else {
                    st.push_back(p);
                }
            }
            ags_need.push_back(p);
            break;
        }
        case ETokens::kOffset: {
            auto* p = static_cast<TOffsetToken*>(raw);
            all.push_back(std::move(token_up));
            if (!st.empty()) {
                if (st.back()->GetType() == ETokens::kOrder) {
                    static_cast<TOrderToken*>(st.back())->offset_ = p;
                } else {
                    st.push_back(p);
                }
            }
            ags_need.push_back(p);
            break;
        }
        case ETokens::kCloseBracket:
        default: {
            if (ags_need.empty()) {
                return MakeError<EError::BadCmdErr>("failed parse query");
            }
            all.push_back(std::move(token_up));
            ags_need.back()->AddArg(raw);
            break;
        }
        }
    }

    return TParsedCommand{std::move(all), std::move(st)};
}

} // namespace JfEngine
