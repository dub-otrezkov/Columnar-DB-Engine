#include "engine.h"

#include "columns/operators/vector_like.h"

namespace JfEngine {

IAoEngine::IAoEngine(std::vector<std::pair<ui64, std::string>> aliases) {
    for (const auto& [i, name] : aliases) {
        if (names.size() <= i) {
            names.resize(i + 1);
        }
        names[i] = name;
    }
}

std::vector<std::string>& IAoEngine::GetNames() {
    return names;
}

TAgregationQuery TAgregationQuery::Clone() {
    std::vector<std::shared_ptr<IOa>> ans(cols.size());
    for (ui64 i = 0; i < cols.size(); i++) {
        ans[i] = cols[i]->Clone();
    }
    for (const auto& [i, j] : edges) {
        ans[i]->AddArg(ans[j].get());
    }
    return TAgregationQuery{std::move(ans), edges};
}

TOperatorQuery TOperatorQuery::Clone() {
    std::vector<std::shared_ptr<IOa>> ans(cols.size());
    for (ui64 i = 0; i < cols.size(); i++) {
        ans[i] = cols[i]->Clone();
    }
    for (const auto& [i, j] : edges) {
        ans[i]->AddArg(ans[j].get());
    }
    return TOperatorQuery{std::move(ans), {}};
}

TAoQuery TAoQuery::Clone() {
    std::vector<std::shared_ptr<IOa>> ans(args.size());
    for (ui64 i = 0; i < ans.size(); i++) {
        ans[i] = args[i]->Clone();
    }
    for (const auto& [i, j] : edges) {
        ans[i]->AddArg(ans[j].get());
    }
    return TAoQuery{{}, std::move(ans), aliases, tp};
}

TOperatorEngine::
    TOperatorEngine(
        TOperatorQuery q,
        std::vector<std::pair<ui64, std::string>> aliases
) :
    IAoEngine(std::move(aliases)),
    q_(std::move(q))
{}

TAgregationEngine::TAgregationEngine(
        TAgregationQuery q,
        std::vector<std::pair<ui64, std::string>> aliases
) :
    IAoEngine(std::move(aliases)),
    q_(std::move(q))
{}

Expected<void> TAgregationEngine::ConsumeRowGroup(ITableInput* inp) {
    bool is_eof = false;
    for (auto& c : q_.cols) {
        auto err = c->ConsumeRowGroup(inp);
        if (err.HasError()) {
            if (err.GetError() == EError::EofErr) {
                is_eof = true;
            } else if (c->is_final) {
                return err.GetError();
            }
        }
    }
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<std::vector<std::shared_ptr<IColumn>>> TAgregationEngine::ThrowRowGroup() {
    std::vector<std::shared_ptr<IColumn>> ans;
    for (auto& c : q_.cols) {
        if (c->is_final) {
            auto [res, _] = c->ThrowRowGroup();
            ans.push_back(std::move(res));
        }
    }
    return std::move(ans);
}

Expected<void> TOperatorEngine::ConsumeRowGroup(ITableInput* inp) {
    bool is_eof = false;
    for (auto& c : q_.cols) {
        auto err = c->ConsumeRowGroup(inp);
        if (err.HasError()) {
            if (err.GetError() == EError::EofErr) {
                is_eof = true;
            } else if (c->is_final) {
                return err.GetError();
            }
        }
    }
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<std::vector<std::shared_ptr<IColumn>>> TOperatorEngine::ThrowRowGroup() {
    std::vector<std::shared_ptr<IColumn>> ans;
    for (auto& c : q_.cols) {
        if (c->is_final) {
            auto [res, _] = c->ThrowRowGroup();
            ans.push_back(std::move(res));
        }
    }
    return std::move(ans);
}

std::shared_ptr<IAoEngine> TOperatorEngine::Clone() {
    std::vector<std::pair<ui64, std::string>> cnames;
    cnames.reserve(names.size());
    for (ui64 i = 0; i < names.size(); i++) {
        cnames.emplace_back(i, names[i]);
    }
    return std::allocate_shared<TOperatorEngine>(ArenaAlloc(), q_.Clone(), std::move(cnames));
}

std::shared_ptr<IAoEngine> TAgregationEngine::Clone() {
    std::vector<std::pair<ui64, std::string>> cnames;
    cnames.reserve(names.size());
    for (ui64 i = 0; i < names.size(); i++) {
        cnames.emplace_back(i, names[i]);
    }
    return std::allocate_shared<TAgregationEngine>(ArenaAlloc(), q_.Clone(), std::move(cnames));
}

std::vector<std::string>& TAgregationEngine::GetNames() {
    names.resize(q_.cols.size());
    ui64 j = 0;
    for (ui64 i = 0; i < q_.cols.size(); i++) {
        if (q_.cols[i]->is_final) {
            if (names[j].empty()) {
                names[j] = q_.cols[i]->GetName();
            }
            j++;
        }
    }
    names.resize(j);
    return names;
}

std::vector<std::string>& TOperatorEngine::GetNames() {
    names.resize(q_.cols.size());
    ui64 j = 0;
    for (ui64 i = 0; i < q_.cols.size(); i++) {
        if (q_.cols[i]->is_final) {
            if (names[j].empty()) {
                names[j] = q_.cols[i]->GetName();
            }
            j++;
        }
    }
    names.resize(j);
    return names;
}

std::shared_ptr<IAoEngine> MakeAoEngine(TAoQuery q) {
    if (q.tp == EAoEngineType::kAgregation) {
        return std::allocate_shared<TAgregationEngine>(
            ArenaAlloc(),
            TAgregationQuery{std::move(q.args), std::move(q.edges)},
            std::move(q.aliases)
        );
    } else { // q.operators.has_value()
        return std::allocate_shared<TOperatorEngine>(
            ArenaAlloc(),
            TOperatorQuery{std::move(q.args), std::move(q.edges)},
            std::move(q.aliases)
        );
    }
}

EAoEngineType TAgregationEngine::GetType() const {
    return EAoEngineType::kAgregation;
}

EAoEngineType TOperatorEngine::GetType() const {
    return EAoEngineType::kOperator;
}

} // namespace JfEngine