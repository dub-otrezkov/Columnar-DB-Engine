#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "utils/errors/errors.h"

namespace JfEngine {

Expected<std::vector<TFilterOp>> ParseWhereConfig(const std::vector<JfEngine::IToken *>& args) {
    std::vector<TFilterOp> config;
    ui64 i = 0;
    while (i < args.size()) {
        if (i + 3 > args.size()) {
            return MakeError<EError::BadCmdErr>();
        }
        if (args[i]->GetType() != ETokens::kNameToken && args[i]->GetType() != ETokens::kAnd) {
            return MakeError<EError::BadCmdErr>();
        }
        if (args[i]->GetType() == ETokens::kAnd) {
            i++;
            continue;
        }
        auto a1 = static_cast<TNameToken*>(args[i]);
        if (args[i + 1]->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>();
        }
        auto a2 = static_cast<TNameToken*>(args[i + 1]);
        if (a2->GetName() == "IN" || (a2->GetName() == "NOT" &&
            args[i + 2]->GetType() == ETokens::kNameToken &&
            static_cast<TNameToken*>(args[i + 2])->GetName() == "IN"))
        {
            bool rev = (a2->GetName() == "NOT");

            i = i + 2 + rev;

            if (args.size() < i + 1) {
                return MakeError<EError::BadCmdErr>();
            }

            std::vector<std::string> args_for_in;

            while (i < args.size() && args[i]->GetType() != ETokens::kCloseBracket) {
                if (args[i]->GetType() == ETokens::kNameToken) {
                    args_for_in.push_back(
                        static_cast<TNameToken*>(args[i])->GetName()
                    );
                }
                i++;
            }
            i++;

            config.emplace_back(
                a1->GetName(),
                (rev ? EFilterType::kNIn : EFilterType::kIn),
                "",
                args_for_in
            );
            continue;
        }
        if (args[i + 2]->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>();
        }
        auto a3 = static_cast<TNameToken*>(args[i + 2]);


        if (a2->GetName() == "NOT") {
            if (i + 4 > args.size()) {
                return MakeError<EError::BadCmdErr>();
            }
            if (args[i + 3]->GetType() != ETokens::kNameToken) {
                return MakeError<EError::BadCmdErr>();
            }
            if (a3->GetName() != "LIKE") {
                return MakeError<EError::BadCmdErr>();
            }
            auto a4 = static_cast<TNameToken*>(args[i + 3]);
            config.emplace_back(
                a1->GetName(),
                EFilterType::kNLike,
                a4->GetName()
            );
            i += 4;
        } else {
            EFilterType op;
            auto ops = a2->GetName();
            if (ops == "=") {
                op = EFilterType::kEq;
            } else if (ops == "<") {
                op = EFilterType::kLess;
            } else if (ops == "<=") {
                op = EFilterType::kLeq;
            } else if (ops == ">") {
                op = EFilterType::kGreater;
            } else if (ops == ">=") {
                op = EFilterType::kGeq;
            } else if (ops == "<>") {
                op = EFilterType::kNeq;
            } else if (ops == "LIKE") {
                op = EFilterType::kLike;
            } else {
                return MakeError<EError::BadCmdErr>();
            }

            config.emplace_back(
                a1->GetName(),
                op,
                a3->GetName()
            );
            i += 3;
        }
    }

    return std::move(config);
}

Expected<TTableInputPtr> TWhereToken::MakeWorker() {
    auto [config, err] = ParseWhereConfig(args_);
    if (err) {
        return err;
    }
    return std::make_shared<TFilter>(
        TIoFactory::GetTableIo(kCurTableInput),
        TFilterQuery{std::move(config)}
    );
}

} // namespace JfEngine
