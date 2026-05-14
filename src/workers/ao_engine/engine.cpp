#include "engine.h"

#include "utils/logger/logger.h"

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
{
    for (auto& c : q_.cols) {
        if (c->is_final) {
            if (auto* col_op = dynamic_cast<TColumnOp*>(c.get())) {
                col_op->is_group_key = true;
            }
        }
    }
}

Expected<void> TAgregationEngine::ConsumeRowGroup(ITableInput* inp, ui64 i) {
    bool is_eof = false;

    // JF_LOG(this, "neww iter" << std::endl);
    for (auto& c : q_.cols) {
        auto err = c->ConsumeRowGroup(inp, i);
        // JF_LOG(this, "got error: " << err.GetError() << " " << c->GetName() << " " << c->GetColumn() << std::endl);
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

std::vector<TColumnPtr> TAgregationEngine::ThrowRowGroup() {
    std::vector<TColumnPtr> ans;
    ui64 sz = 0;
    for (auto& c : q_.cols) {
        if (c->is_final) {
            ans.push_back(c->ThrowRowGroup());
            sz = std::max(sz, ans.back()->GetSize());
            // JF_LOG(nullptr, "res column size " << ans.back()->GetSize() << std::endl);
        }
    }
    ui64 i = 0;
    for (auto& c : q_.cols) {
        if (c->is_final) {
            if (c->IsConst()) {
                // ans.push_back(nullptr);
                Do<OCloneConst>(ans.at(i), sz);
            }
            i++;
            // JF_LOG(nullptr, "res column size " << ans.back()->GetSize() << std::endl);
        }
    }
    return ans;
}

Expected<void> TOperatorEngine::ConsumeRowGroup(ITableInput* inp, ui64 i) {
    bool is_eof = false;
    for (auto& c : q_.cols) {
        auto err = c->ConsumeRowGroup(inp, i);
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

std::vector<TColumnPtr> TOperatorEngine::ThrowRowGroup() {
    std::vector<TColumnPtr> ans;
    for (auto& c : q_.cols) {
        if (c->is_final) {
            ans.push_back(c->ThrowRowGroup());
        }
    }
    return ans;
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
        return std::make_shared<TAgregationEngine>(
            TAgregationQuery{std::move(q.args), std::move(q.edges)},
            std::move(q.aliases)
        );
    } else { // q.operators.has_value()
        return std::make_shared<TOperatorEngine>(
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