#include "engine.h"

#include "utils/logger/logger.h"

#include "columns/operators/vector_like.h"

namespace JfEngine {

IAoEngine::IAoEngine(
    std::vector<std::unique_ptr<IOa>> cols,
    std::vector<std::pair<ui64, std::string>> aliases
) :
    cols_(std::move(cols))
{
    for (const auto& [i, name] : aliases) {
        if (names_.size() <= i) {
            names_.resize(i + 1);
        }
        names_.at(i) = name;
    }
}

// abababadkkddkkdkdkdkdkdkkdkdkdkdkdkdkdkdkkd
// ababcbcdkdjksldklsdkddkhjfdjhfdkfjkdjfdkjfd

TOperatorEngine::
    TOperatorEngine(
        std::vector<std::unique_ptr<IOa>> q,
        std::vector<std::pair<ui64, std::string>> aliases
) :
    IAoEngine(std::move(q), std::move(aliases))
{}

TAgregationEngine::TAgregationEngine(
        std::vector<std::unique_ptr<IOa>> q,
        std::vector<std::pair<ui64, std::string>> aliases
) :
    IAoEngine(std::move(q), std::move(aliases))
{
    for (auto& c : cols_) {
        if (c->is_final) {
            if (auto* col_op = dynamic_cast<TColumnOp*>(c.get())) {
                col_op->is_group_key = true;
            }
        }
    }
}

std::vector<std::string>& IAoEngine::GetNames() {
    names_.resize(cols_.size());
    ui64 j = 0;
    for (ui64 i = 0; i < cols_.size(); i++) {
        if (cols_[i]->is_final) {
            if (names_[j].empty()) {
                names_[j] = cols_[i]->GetName();
            }
            j++;
        }
    }
    names_.resize(j);
    return names_;
}

Expected<void> IAoEngine::ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* i) {
    bool is_eof = false;
    // JF_LOG(this, "new iengine consumtion" << std::endl);
    for (auto& c : cols_) {
        // JF_LOG(this, "col name " << c->GetName() << " " << c->GetColumn() << std::endl);
        auto err = c->ConsumeRowGroup(inp, i);
        // JF_LOG(this, "col " << err.GetError() << std::endl);
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

std::vector<TColumnPtr> IAoEngine::ThrowRowGroup() {
    std::vector<TColumnPtr> ans;
    ui64 sz = 0;
    for (auto& c : cols_) {
        if (c->is_final) {
            ans.push_back(c->ThrowRowGroup());
            sz = std::max(sz, ans.back()->GetSize());
        }
    }
    ui64 i = 0;
    for (auto& c : cols_) {
        if (c->is_final) {
            if (c->IsConst()) {
                Do<OCloneConst>(ans.at(i), sz);
            }
            i++;
        }
    }
    return ans;
}

std::shared_ptr<IAoEngine> MakeAoEngine(TAoQuery q) {
    if (q.tp == EAoEngineType::kAgregation) {
        return std::make_shared<TAgregationEngine>(
            std::move(q.args),
            std::move(q.aliases)
        );
    } else { // q.operators.has_value()
        return std::make_shared<TOperatorEngine>(
            std::move(q.args),
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