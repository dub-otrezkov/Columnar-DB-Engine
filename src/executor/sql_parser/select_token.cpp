#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

#include <functional>

namespace JfEngine {

TAoQuery ParseArgs(std::vector<std::shared_ptr<IToken>> inp) {

    std::vector<std::pair<ui64, std::string>> aliases;

    bool next_alias = false;

    std::vector<std::unique_ptr<IOa>> args;
    std::vector<IOa*> st;

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

                auto node = std::make_unique<TColumnOp>(d);

                auto ptr = node.get();

                if (!st.empty()) {
                    st.back()->AddArg(std::move(node));
                } else {
                    args.push_back(std::move(node));
                }

                break;
            }
            case ETokens::kSum: {
                auto node = std::make_unique<TSumAgr>();
                etype = EAoEngineType::kAgregation;

                auto ptr = node.get();

                args.push_back(std::move(node));

                st.push_back(ptr);

                break;
            }
            case ETokens::kCount: {
                auto node = std::make_unique<TCountAgr>();
                etype = EAoEngineType::kAgregation;

                auto ptr = node.get();

                args.push_back(std::move(node));

                st.push_back(ptr);

                break;
            }
            case ETokens::kAvg: {
                auto node = std::make_unique<TAvgAgr>();
                etype = EAoEngineType::kAgregation;

                auto ptr = node.get();

                args.push_back(std::move(node));

                st.push_back(ptr);

                break;
            }
            case ETokens::kMin: {
                auto node = std::make_unique<TMinAgr>();
                etype = EAoEngineType::kAgregation;

                auto ptr = node.get();

                args.push_back(std::move(node));

                st.push_back(ptr);

                break;
            }
            case ETokens::kMax: {
                auto node = std::make_unique<TMaxAgr>();
                etype = EAoEngineType::kAgregation;

                auto ptr = node.get();

                args.push_back(std::move(node));

                st.push_back(ptr);

                break;
            }
            case ETokens::kDistinct: {
                auto node = std::make_unique<TDistinctOp>();

                auto ptr = node.get();

                if (!st.empty()) {
                    st.back()->AddArg(std::move(node));
                } else {
                    args.push_back(std::move(node));
                }

                st.push_back(ptr);

                break;
            }
            case ETokens::kLength: {
                auto node = std::make_unique<TLengthOp>();

                auto ptr = node.get();

                if (!st.empty()) {
                    st.back()->AddArg(std::move(node));
                } else {
                    args.push_back(std::move(node));
                }

                st.push_back(ptr);

                break;
            }
            case ETokens::kPlus: {
                auto node = std::make_unique<TPlusOp>();

                auto ptr = node.get();

                if (!st.empty()) {
                    st.back()->AddArg(std::move(node));
                } else {
                    args.push_back(std::move(node));
                }

                st.push_back(ptr);

                break;
            }
            case ETokens::kMinus: {
                auto node = std::make_unique<TMinusOp>();

                auto ptr = node.get();

                if (!st.empty()) {
                    st.back()->AddArg(std::move(node));
                } else {
                    args.push_back(std::move(node));
                }

                st.push_back(ptr);

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

Expected<ITableInput> TSelectToken::MakeWorker() {
    if (!is_id_) {
        auto args = ParseArgs(args_);

        TIoFactory::GetTableIo(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput).GetShared(),
            std::move(args)
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

        eng.WriteDataToCsv(TIoFactory::GetIo(kResultData).GetRes());
        eng.WriteSchemeToCsv(TIoFactory::GetIo(kResultScheme).GetRes());

        return nullptr;
    }
}

} // namespace JfEngine