#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

namespace JFEngine {

ETokens TSelectToken::GetType() const {
    return ETokens::kSelect;
}


std::vector<std::shared_ptr<IAgregation>> TSelectToken::ParseArgs() {
    std::vector<std::shared_ptr<IAgregation>> args;
    std::vector<std::shared_ptr<IAgregation>> st;

    for (auto& token : args_) {
        switch (token->GetType()) {
            case ETokens::kCloseBracket: {
                st.pop_back();
                break;
            }
            case ETokens::kNameToken: {
                auto d = static_cast<TNameToken*>(token.get())->GetName();
                // std::cout << "! " << d << std::endl;

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
        }
    }
    return std::move(args);
}

void TSelectToken::SetIsId() {
    is_id_ = true;
}

Expected<ITableInput> TSelectToken::Exec() {
    if (!is_id_) {
        auto args = ParseArgs();

        TIOFactory::GetTableIO(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIOFactory::GetTableIO(kCurTableInput).GetShared(),
            TGlobalAgregationQuery{args}
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

        std::cout << "fkfkfkf" << std::endl;

        auto agr = std::make_shared<TAgregator>(
            TIOFactory::GetTableIO(kCurTableInput).GetShared()
        );

        TEngine eng;

        TIOFactory::RegisterFileIO(kResultScheme, ETypeFile::kCSVFile);
        TIOFactory::RegisterFileIO(kResultData, ETypeFile::kCSVFile);

        eng.Setup(agr);

        eng.WriteDataToCSV(TIOFactory::GetIO(kResultData).GetRes());
        eng.WriteSchemeToCSV(TIOFactory::GetIO(kResultScheme).GetRes());

        return nullptr;
    }
}

} // namespace JFEngine