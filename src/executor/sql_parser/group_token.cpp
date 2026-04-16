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
            std::unique_ptr<IOa> col_n = nullptr;
            ui64 k = 0;
            for (const auto& [j, name] : selects_.aliases) {
                if (j == i) {
                    col_n = std::make_unique<TColumnOp>(name);
                    qop.aliases.emplace_back(j, name);
                    selects_.aliases.erase(selects_.aliases.begin() + k);
                    break;
                }
                k++;
            }
            if (!col_n) {
                col_n = std::make_unique<TColumnOp>(col->GetName());
            }

            std::swap(col, col_n);
            qop.args.push_back(std::move(col_n));
        } else {
            IAgregationOnly* casted = static_cast<IAgregationOnly*>(col.get());
            std::unique_ptr<IOa> col_n = std::make_unique<TColumnOp>(casted->arg->GetName());
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
        std::move(query),
        std::move(selects_)
    );
}

} // namespace JfEngine
