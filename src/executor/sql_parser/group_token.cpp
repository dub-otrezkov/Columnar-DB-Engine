#include "tokenizer.h"

#include "memory/arena.h"

#include "workers/groupby/groupby.h"
#include "workers/global_agregations/agregator.h"
#include "ios_factory/ios_factory.h"

#include <boost/unordered/unordered_flat_set.hpp>

namespace JfEngine {

Expected<ITableInput> TGroupToken::MakeWorker() {
    args_.erase(args_.begin());
    auto q = ParseArgs(args_);

    boost::unordered_flat_set<std::string> as;
    for (const auto& [_, name] : q.aliases) {
        as.insert(name);
    }

    TGroupByQuery query;
    query.cols = std::move(q.args);

    if (limit_) {
        query.limit = limit_->GetLimit();
    }

    TAoQuery qop;
    qop.tp = EAoEngineType::kOperator;

    ui64 i = 0;
    boost::unordered_flat_set<std::string> used;
    for (auto& col : selects_.args) {
        if (!col->is_final) {
            continue;
        }
        if (col->GetType() == EAoType::kOperator) {
            std::shared_ptr<IOa> col_n = nullptr;
            ui64 k = 0;
            for (const auto& [j, name] : selects_.aliases) {
                if (j == i) {
                    col_n = std::allocate_shared<TColumnOp>(ArenaAlloc(), name);
                    qop.aliases.emplace_back(j, name);
                    selects_.aliases.erase(selects_.aliases.begin() + k);
                    break;
                }
                k++;
            }
            if (!col_n) {
                col_n = std::allocate_shared<TColumnOp>(ArenaAlloc(), col->GetName());
            }

            if (col->is_final) {
                col_n->is_final = true;
            }

            std::swap(col, col_n);
            used.insert(col_n->GetName());
            qop.args.push_back(std::move(col_n));
        } else {
            qop.args.push_back(std::allocate_shared<TColumnOp>(ArenaAlloc(), col->GetColumn()));
            qop.args.back()->is_final = true;
            used.insert(qop.args.back()->GetName());
        }
        i++;
    }

    for (auto& agr : query.cols) {
        auto name = agr->GetName();
        std::cout << name << std::endl;
        if (as.contains(name) || used.contains(name) || name == "*") {
            continue;
        }
        qop.args.push_back(std::make_shared<TColumnOp>(name));
        used.insert(qop.args.back()->GetName());
    }

    for (auto& p : query.cols) {
        p->is_final = true;
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
