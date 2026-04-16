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
    std::vector<std::unique_ptr<IOa>> ans(cols.size());
    for (ui64 i = 0; i < cols.size(); i++) {
        ans[i] = cols[i]->Clone();
    }
    return TAgregationQuery{std::move(ans)};
}

TOperatorQuery TOperatorQuery::Clone() {
    std::vector<std::unique_ptr<IOa>> ans(cols.size());
    for (ui64 i = 0; i < cols.size(); i++) {
        ans[i] = cols[i]->Clone();
    }
    return TOperatorQuery{std::move(ans)};
}

TAoQuery TAoQuery::Clone() {
    std::vector<std::unique_ptr<IOa>> ans(args.size());
    for (ui64 i = 0; i < ans.size(); i++) {
        ans[i] = args[i]->Clone();
    }
    return TAoQuery{std::move(ans), aliases, tp};
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
            } else {
                return err.GetError();
            }
        }
    }
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<std::vector<std::shared_ptr<IColumn>>> TAgregationEngine::ThrowRowGroup() {
    std::vector<std::shared_ptr<IColumn>> ans;
    for (auto& c : q_.cols) {
        auto [res, _] = c->ThrowRowGroup();
        ans.push_back(std::move(res));
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
            } else {
                return err.GetError();
            }
        }
    }
    return (is_eof ? EError::EofErr : EError::NoError);
}

Expected<std::vector<std::shared_ptr<IColumn>>> TOperatorEngine::ThrowRowGroup() {
    std::vector<std::shared_ptr<IColumn>> ans;
    for (auto& c : q_.cols) {
        auto [res, _] = c->ThrowRowGroup();
        ans.push_back(std::move(res));
    }
    return std::move(ans);
}

std::shared_ptr<IAoEngine> TOperatorEngine::Clone() {
    static std::vector<std::pair<ui64, std::string>> cnames;
    names.clear();
    for (ui64 i = 0; i < names.size(); i++) {
        cnames.emplace_back(i, names[i]);
    }
    return std::make_shared<TOperatorEngine>(q_.Clone(), cnames);
}

std::shared_ptr<IAoEngine> TAgregationEngine::Clone() {
    static std::vector<std::pair<ui64, std::string>> cnames;
    names.clear();
    for (ui64 i = 0; i < names.size(); i++) {
        cnames.emplace_back(i, names[i]);
    }
    return std::make_shared<TAgregationEngine>(q_.Clone(), cnames);
}

std::vector<std::string>& TAgregationEngine::GetNames() {
    names.resize(q_.cols.size());
    for (ui64 i = 0; i < q_.cols.size(); i++) {
        if (names[i].empty()) {
            names[i] = q_.cols[i]->GetName();
        }
    }
    return names;
}

std::vector<std::string>& TOperatorEngine::GetNames() {
    names.resize(q_.cols.size());
    for (ui64 i = 0; i < q_.cols.size(); i++) {
        if (names[i].empty()) {
            names[i] = q_.cols[i]->GetName();
        }
    }
    return names;
}

std::shared_ptr<IAoEngine> MakeAoEngine(TAoQuery q) {

    if (q.tp == EAoEngineType::kAgregation) {
        return std::make_shared<TAgregationEngine>(
            TAgregationQuery{std::move(q.args)},
            std::move(q.aliases)
        );
    } else { // q.operators.has_value()
        return std::make_shared<TOperatorEngine>(
            TOperatorQuery{std::move(q.args)},
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