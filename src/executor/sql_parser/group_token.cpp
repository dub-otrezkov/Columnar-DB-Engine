#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "memory/arena.h"
#include "workers/global_agregations/agregator.h"
#include "workers/groupby/groupby.h"

#include <boost/unordered/unordered_flat_set.hpp>

namespace JfEngine {

Expected<TTableInputPtr> TGroupToken::MakeWorker() {
    args_.erase(args_.begin());
    auto q = ParseArgs(args_);

    boost::unordered_flat_set<std::string> as;
    for (const auto& [_, name] : q.aliases) {
        as.insert(name);
    }

    TGroupByQuery query;
    for (auto& col : q.args) {
        if (!col->IsConst() && col->is_final) {
            query.cols.push_back(std::move(col));
        }
    }

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
    ui64 jrank = 0;
    for (ui64 i = 0; i < selects_.args.size(); i++) {
        auto& col = selects_.args[i];
        if (!col->is_final && !in_op_subtree[i] || col->IsConst()) {
            continue;
        }
        if (col->GetType() == EAoType::kOperator) {
            std::unique_ptr<IOa> col_n = nullptr;
            ui64 k = 0;
            for (const auto& [j, name] : selects_.aliases) {
                if (col->is_final && j == jrank) {
                    col_n = std::make_unique<TColumnOp>(name);
                    qop.aliases.emplace_back(jrank, name);
                    selects_.aliases.erase(selects_.aliases.begin() + k);
                    break;
                }
                k++;
            }
            if (!col_n) {
                col_n = std::make_unique<TColumnOp>(col->GetName());
            }

            col_n->is_final = col->is_final;

            col.swap(col_n);
            used.insert(col_n->GetName());
            qop.args.push_back(std::move(col_n));
            if (col->is_final) {
                jrank++;
            }
        } else {
            auto col_name = col->GetColumn();
            if (!used.contains(col_name)) {
                qop.args.push_back(std::make_unique<TColumnOp>(col_name));
                qop.args.back()->is_final = col->is_final;
                used.insert(col_name);
            }
            if (col->is_final) {
                jrank++;
            }
        }
    }

    for (auto& agr : query.cols) {
        auto name = agr->GetName();
        if (as.contains(name) || used.contains(name) || name == "*" || agr->IsConst()) {
            continue;
        }
        qop.args.push_back(std::make_unique<TColumnOp>(name));
        used.insert(qop.args.back()->GetName());
    }

    return std::make_shared<TGroupBy>(
        std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput),
            std::move(qop)
        ),
        std::move(query),
        std::move(selects_)
    );
}

} // namespace JfEngine
