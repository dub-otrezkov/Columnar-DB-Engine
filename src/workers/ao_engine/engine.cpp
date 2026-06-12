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
    for (auto& c : cols_) {
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

Expected<std::vector<TColumnPtr>> IAoEngine::AgregateFromMeta(ITableInput*) {
    return MakeError<EError::UnsupportedErr>();
}

Expected<std::vector<TColumnPtr>> TAgregationEngine::AgregateFromMeta(ITableInput* inp) {
    std::vector<std::string> sum_cols;
    for (auto& c : cols_) {
        if (!c->is_final) {
            continue;
        }
        auto* sa = dynamic_cast<TSumAgr*>(c.get());
        if (!sa) {
            return MakeError<EError::UnsupportedErr>();
        }
        auto* co = dynamic_cast<TColumnOp*>(sa->arg);
        if (!co) {
            return MakeError<EError::UnsupportedErr>();
        }
        sum_cols.push_back(co->GetColumn());
    }
    if (sum_cols.empty()) {
        return MakeError<EError::UnsupportedErr>();
    }

    std::vector<i128> totals(sum_cols.size(), 0);
    bool run = true;
    for (; run; inp->MoveCursor()) {
        for (ui64 k = 0; k < sum_cols.size(); k++) {
            auto r = inp->ReadSum(sum_cols[k]);
            if (r.HasError() && r.GetError() != EError::EofErr) {
                return r.GetError();
            }
            totals[k] += static_cast<Ti128Column*>(r.GetRes().get())->GetData().at(0);
            if (r.GetError() == EError::EofErr) {
                run = false;
            }
        }
    }

    std::vector<TColumnPtr> ans;
    ans.reserve(sum_cols.size());
    for (auto t : totals) {
        ans.push_back(std::make_shared<Ti128Column>(std::vector<i128>{t}));
    }
    return ans;
}

std::shared_ptr<IAoEngine> MakeAoEngine(TAoQuery q) {
    if (q.tp == EAoEngineType::kAgregation) {
        return std::make_shared<TAgregationEngine>(
            std::move(q.args),
            std::move(q.aliases)
        );
    } else {
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
