#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "memory/arena.h"
#include "workers/global_agregations/agregator.h"
#include "workers/groupby/groupby.h"

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

    std::vector<std::vector<ui64>> adj(selects_.args.size());
    for (const auto& [p, c] : selects_.edges) {
        adj[p].push_back(c);
    }
    std::vector<bool> in_op_subtree(selects_.args.size(), false);
    {
        std::vector<ui64> stack;
        for (ui64 k = 0; k < selects_.args.size(); k++) {
            if (selects_.args[k]->is_final && selects_.args[k]->GetType() != EAoType::kAgregation) {
                stack.push_back(k);
            }
        }
        while (!stack.empty()) {
            ui64 v = stack.back();
            stack.pop_back();
            if (in_op_subtree[v]) {
                continue;
            }
            in_op_subtree[v] = true;
            for (ui64 u : adj[v]) {
                stack.push_back(u);
            }
        }
    }

    boost::unordered_flat_set<std::string> used;
    for (ui64 i = 0; i < selects_.args.size(); i++) {
        auto& col = selects_.args[i];
        if (!col->is_final && !in_op_subtree[i]) {
            continue;
        }
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

            col_n->is_final = col->is_final;

            // std::swap(col, col_n);
            col.swap(col_n);
            used.insert(col_n->GetName());
            qop.args.push_back(std::move(col_n));
        } else {
            qop.args.push_back(std::make_shared<TColumnOp>(col->GetColumn()));
            qop.args.back()->is_final = col->is_final;
            used.insert(qop.args.back()->GetName());
        }
    }

    for (auto& agr : query.cols) {
        auto name = agr->GetName();
        if (as.contains(name) || used.contains(name) || name == "*") {
            continue;
        }
        qop.args.push_back(std::make_shared<TColumnOp>(name));
        used.insert(qop.args.back()->GetName());
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
