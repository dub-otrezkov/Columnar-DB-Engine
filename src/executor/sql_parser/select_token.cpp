#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

namespace JFEngine {

ETokens TSelectToken::GetType() const {
    return ETokens::kSelect;
}

Expected<ITableInput> TSelectToken::Exec() {
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
<<<<<<< HEAD
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
=======
>>>>>>> main
        }
    }
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
}

} // namespace JFEngine