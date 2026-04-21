#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "utils/errors/errors.h"
#include "workers/filters/filter.h"

namespace JfEngine {

Expected<ITableInput> TWhereToken::MakeWorker() {
    std::vector<TFilterOp> config;
    ui64 i = 0;
    while (i < args_.size()) {
        if (i + 3 > args_.size()) {
            return MakeError<EError::BadCmdErr>();
        }
        if (args_[i]->GetType() != ETokens::kNameToken && args_[i]->GetType() != ETokens::kAnd) {
            return MakeError<EError::BadCmdErr>();
        }
        if (args_[i]->GetType() == ETokens::kAnd) {
            i++;
            continue;
        }
        auto a1 = static_cast<TNameToken*>(args_[i].get());
        if (args_[i + 1]->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>();
        }
        auto a2 = static_cast<TNameToken*>(args_[i + 1].get());
        if (a2->GetName() == "IN" || (a2->GetName() == "NOT" &&
            args_[i + 2].get()->GetType() == ETokens::kNameToken && 
            static_cast<TNameToken*>(args_[i + 2].get())->GetName() == "IN"))
        {
            bool rev = (a2->GetName() == "NOT");

            i = i + 2 + rev;

            if (args_.size() < i + 1) {
                return MakeError<EError::BadCmdErr>();
            }

            std::vector<std::string> args_for_in;

            while (i < args_.size() && args_[i]->GetType() != ETokens::kCloseBracket) {
                if (args_[i]->GetType() == ETokens::kNameToken) {
                    args_for_in.push_back(
                        static_cast<TNameToken*>(args_[i].get())->GetName()
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
        if (args_[i + 2]->GetType() != ETokens::kNameToken) {
            return MakeError<EError::BadCmdErr>();
        }
        auto a3 = static_cast<TNameToken*>(args_[i + 2].get());


        if (a2->GetName() == "NOT") {
            if (i + 4 > args_.size()) {
                return MakeError<EError::BadCmdErr>();
            }
            if (args_[i + 3]->GetType() != ETokens::kNameToken) {
                return MakeError<EError::BadCmdErr>();
            }
            if (a3->GetName() != "LIKE") {
                return MakeError<EError::BadCmdErr>();
            }
            auto a4 = static_cast<TNameToken*>(args_[i + 3].get());
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

    return std::make_shared<TFilter>(
        TIoFactory::GetTableIo(kCurTableInput).GetShared(),
        TFilterQuery{std::move(config)}
    );
}

} // namespace JfEngine
