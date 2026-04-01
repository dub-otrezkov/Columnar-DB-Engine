#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

namespace JFEngine {

ETokens TSelectToken::GetType() const {
    return ETokens::kSelect;
}

TGlobalAgregationQuery ParseArgs(std::vector<std::shared_ptr<IToken>> inp) {
    std::vector<std::shared_ptr<IAgregation>> args;
    std::vector<std::shared_ptr<IAgregation>> st;

    std::vector<std::pair<ui64, std::string>> aliases;

    bool next_alias = false;

    for (auto& token : inp) {
        if (next_alias) {
            next_alias = false;
            if (token->GetType() == ETokens::kNameToken) {
                aliases.emplace_back(
                    args.size() - 1,
                    static_cast<TNameToken*>(token.get())->GetName()
                );
                continue;
            }
        }
        switch (token->GetType()) {
            case ETokens::kCloseBracket: {
                st.pop_back();
                break;
            }
            case ETokens::kAs: {
                next_alias = true;
                break;
            }
            case ETokens::kNameToken: {
                auto d = static_cast<TNameToken*>(token.get())->GetName();

                auto node = std::make_shared<TColumnAgr>(d);

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                break;
            }
            case ETokens::kSum: {
                auto node = std::make_shared<TSumAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kCount: {
                auto node = std::make_shared<TCountAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kAvg: {
                auto node = std::make_shared<TAvgAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kMin: {
                auto node = std::make_shared<TMinAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kMax: {
                auto node = std::make_shared<TMaxAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kDistinct: {
                auto node = std::make_shared<TDistinctAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kLength: {
                auto node = std::make_shared<TLengthAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kPlus: {
                auto node = std::make_shared<TPlusAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kMinus: {
                auto node = std::make_shared<TMinusAgr>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
        }
    }
    return TGlobalAgregationQuery{
        std::move(args),
        std::move(aliases)
    };
}

void TSelectToken::SetIsId() {
    is_id_ = true;
}

std::vector<std::shared_ptr<IToken>> TSelectToken::GetArgs() {
    return args_;
}

Expected<ITableInput> TSelectToken::Exec() {
    if (!is_id_) {
        auto args = ParseArgs(args_);

        TIOFactory::GetTableIO(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIOFactory::GetTableIO(kCurTableInput).GetShared(),
            args
        );

        TEngine eng;

        TIOFactory::RegisterFileIO(kResultScheme, ETypeFile::kCSVFile);
        TIOFactory::RegisterFileIO(kResultData, ETypeFile::kCSVFile);

        eng.Setup(agr);

        eng.WriteDataToCSV(TIOFactory::GetIO(kResultData).GetRes());
        eng.WriteSchemeToCSV(TIOFactory::GetIO(kResultScheme).GetRes());

        return nullptr;
    } else {
        TIOFactory::GetTableIO(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIOFactory::GetTableIO(kCurTableInput).GetShared()
        );

        TEngine eng;

        TIOFactory::RegisterFileIO(kResultScheme, ETypeFile::kCSVFile);
        TIOFactory::RegisterFileIO(kResultData, ETypeFile::kCSVFile);

        eng.Setup(agr);

        // std::cout << ":: " << TIOFactory::GetIO(kResultData).GetShared() << " ";
        // std::cout << TIOFactory::GetIO(kResultScheme).GetShared() << std::endl;
        eng.WriteDataToCSV(TIOFactory::GetIO(kResultData).GetRes());
        eng.WriteSchemeToCSV(TIOFactory::GetIO(kResultScheme).GetRes());

        return nullptr;
    }
}

} // namespace JFEngine