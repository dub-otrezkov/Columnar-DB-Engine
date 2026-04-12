#include "tokenizer.h"

#include "workers/groupby/groupby.h"
#include "workers/global_agregations/agregator.h"
#include "ios_factory/ios_factory.h"

namespace JfEngine {

Expected<ITableInput> TGroupToken::MakeWorker() {
    args_.erase(args_.begin());
    auto q = ParseArgs(args_);
    TGroupByQuery query;
    query.cols = std::move(q.args);

    if (limit_) {
        query.limit = limit_->GetLimit();
    }

    TAoQuery qop;
    qop.tp = EAoEngineType::kOperator;

    ui64 i = 0;
    for (auto& col : selects_.args) {
        if (col->GetType() == EAoType::kOperator) {
            std::shared_ptr<IOa> col_n = nullptr;
            ui64 k = 0;
            for (const auto& [j, name] : selects_.aliases) {
                if (j == i) {
                    col_n = std::make_shared<TColumnOp>(name);
                    qop.aliases.emplace_back(j, name);
                    selects_.aliases.erase(selects_.aliases.begin() + k);
                    break;
                }
                k++;
            }
            if (!col_n) {
                col_n = std::make_shared<TColumnOp>(col->GetName());
            }

            std::swap(col, col_n);
            qop.args.push_back(std::move(col_n));
        } else {
            auto casted = std::static_pointer_cast<IAgregationOnly>(col);
            std::shared_ptr<IOa> col_n = std::make_shared<TColumnOp>(casted->arg->GetName());
            std::swap(casted->arg, col_n);
            qop.args.push_back(std::move(col_n));
        }
        i++;
    }

    return std::make_shared<TGroupBy>(
        std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput).GetShared(),
            std::move(qop)
        ),
        query,
        selects_
    );
}

void TGroupToken::SetSelects(TAoQuery s) {
    selects_ = std::move(s);
}

ETokens TGroupToken::GetType() const {
    return ETokens::kGroup;
}

} // namespace JfEngine
