#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "memory/arena.h"
#include "workers/global_agregations/agregator.h"
#include "workers/groupby/groupby.h"

#include "utils/logger/logger.h"

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
            query.cols.push_back(col->GetName());
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

    // Names that qop will OUTPUT in its schema (= narrow's scheme).
    // - For kOp: NEW TColumnOp's name after swap (alias if matched, else original).
    // - For kAg: name of the TColumnOp pushed for the underlying column.
    boost::unordered_flat_set<std::string> qop_output_names;

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
            // col (in selects_) is now NEW TColumnOp with the qop-output name.
            qop_output_names.insert(col->GetName());
            qop.args.push_back(std::move(col_n));
            if (col->is_final) {
                jrank++;
            }
        } else {
            qop.args.push_back(std::make_unique<TColumnOp>(col->GetColumn()));
            qop.args.back()->is_final = col->is_final;
            qop_output_names.insert(qop.args.back()->GetName());
            if (col->is_final) {
                jrank++;
            }
        }
    }

    for (auto& name : query.cols) {
        if (as.contains(name) || qop_output_names.contains(name) || name == "*") {
            continue;
        }
        qop.args.push_back(std::make_unique<TColumnOp>(name));
        qop.args.back()->is_final = true;
        qop_output_names.insert(name);
    }

    // JF_LOG(this, "qop.args.size()=" << qop.args.size() << " etype=" << (int)qop.tp);
    // for (ui64 i = 0; i < qop.args.size(); i++) {
    //     JF_LOG(this, "  qop[" << i << "] name=" << qop.args[i]->GetName()
    //                 << " col=" << qop.args[i]->GetColumn()
    //                 << " final=" << qop.args[i]->is_final
    //                 << " type=" << (qop.args[i]->GetType() == EAoType::kAgregation ? "AGR" : "OP"));
    // }
    // JF_LOG(this, "selects_.args.size()=" << selects_.args.size());
    // for (ui64 i = 0; i < selects_.args.size(); i++) {
    //     JF_LOG(this, "  sel[" << i << "] name=" << selects_.args[i]->GetName()
    //                 << " col=" << selects_.args[i]->GetColumn()
    //                 << " final=" << selects_.args[i]->is_final
    //                 << " in_op_subtree=" << (i < in_op_subtree.size() ? in_op_subtree[i] : false));
    // }
    // JF_LOG(this, "selects_.aliases.size()=" << selects_.aliases.size());
    // for (const auto& [j, n] : selects_.aliases) {
    //     JF_LOG(this, "  alias j=" << j << " name=" << n);
    // }


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
