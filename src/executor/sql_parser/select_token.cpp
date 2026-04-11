#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

#include <functional>

namespace JfEngine {

ETokens TSelectToken::GetType() const {
    return ETokens::kSelect;
}

TAoQuery ParseArgs(std::vector<std::shared_ptr<IToken>> inp) {

    std::vector<std::pair<ui64, std::string>> aliases;

    bool next_alias = false;

    std::vector<std::shared_ptr<IOa>> args;
    std::vector<std::shared_ptr<IOa>> st;

    EAoEngineType etype = EAoEngineType::kOperator;

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

                auto node = std::make_shared<TColumnOp>(d);

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                break;
            }
            case ETokens::kSum: {
                auto node = std::make_shared<TSumAgr>();
                etype = EAoEngineType::kAgregation;

                args.push_back(node);

                st.push_back(node);

                break;
            }
            case ETokens::kCount: {
                auto node = std::make_shared<TCountAgr>();
                etype = EAoEngineType::kAgregation;

                args.push_back(node);

                st.push_back(node);

                break;
            }
            case ETokens::kAvg: {
                auto node = std::make_shared<TAvgAgr>();
                etype = EAoEngineType::kAgregation;

                args.push_back(node);

                st.push_back(node);

                break;
            }
            case ETokens::kMin: {
                auto node = std::make_shared<TMinAgr>();
                etype = EAoEngineType::kAgregation;
                
                args.push_back(node);

                st.push_back(node);

                break;
            }
            case ETokens::kMax: {
                auto node = std::make_shared<TMaxAgr>();
                etype = EAoEngineType::kAgregation;

                args.push_back(node);

                st.push_back(node);

                break;
            }
            case ETokens::kDistinct: {
                auto node = std::make_shared<TDistinctOp>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kLength: {
                auto node = std::make_shared<TLengthOp>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kPlus: {
                auto node = std::make_shared<TPlusOp>();

                if (!st.empty()) {
                    st.back()->AddArg(node);
                } else {
                    args.push_back(node);
                }

                st.push_back(node);

                break;
            }
            case ETokens::kMinus: {
                auto node = std::make_shared<TMinusOp>();

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
    return TAoQuery{
        std::move(args),
        std::move(aliases),
        etype
    };
}

void TSelectToken::SetIsId() {
    is_id_ = true;
}

std::vector<std::shared_ptr<IToken>> TSelectToken::GetArgs() {
    return args_;
}

Expected<ITableInput> TSelectToken::MakeWorker() {
    if (!is_id_) {
        auto args = ParseArgs(args_);

        TIoFactory::GetTableIo(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput).GetShared(),
            args
        );

        TEngine eng;

        TIoFactory::RegisterFileIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(kResultData, ETypeFile::kCsvFile);

        eng.Setup(agr);

        eng.WriteDataToCsv(TIoFactory::GetIo(kResultData).GetRes());
        eng.WriteSchemeToCsv(TIoFactory::GetIo(kResultScheme).GetRes());

        return nullptr;
    } else {
        TIoFactory::GetTableIo(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput).GetShared()
        );

        TEngine eng;

        TIoFactory::RegisterFileIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(kResultData, ETypeFile::kCsvFile);

        eng.Setup(agr);

        // std::cout << ":: " << TIoFactory::GetIo(kResultData).GetShared() << " ";
        // std::cout << TIoFactory::GetIo(kResultScheme).GetShared() << std::endl;
        eng.WriteDataToCsv(TIoFactory::GetIo(kResultData).GetRes());
        eng.WriteSchemeToCsv(TIoFactory::GetIo(kResultScheme).GetRes());

        return nullptr;
    }
}

} // namespace JfEngine