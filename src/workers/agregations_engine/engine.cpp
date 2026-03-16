#include "engine.h"

#include "columns/operators/vector_like.h"

namespace JFEngine {

TGlobalAgregationQuery TGlobalAgregationQuery::Clone() {
    std::vector<std::shared_ptr<IAgregation>> ans(cols.size());
    for (ui64 i = 0; i < cols.size(); i++) {
        ans[i] = cols[i]->Clone();
    }
    return TGlobalAgregationQuery{std::move(ans)};
}

TAgregationsEngine::TAgregationsEngine(TGlobalAgregationQuery qry, bool groupby) :
    cols_(std::move(qry.cols)),
    aliases_(std::move(qry.aliases)),
    is_groupby_(groupby)
{}

Expected<void> TAgregationsEngine::ConsumeRowGroup(ITableInput* inp) {
    bool is_eof = false;
    for (auto& c : cols_) {
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

Expected<std::vector<std::shared_ptr<IColumn>>> TAgregationsEngine::ThrowRowGroup() {
    std::vector<std::shared_ptr<IColumn>> ans;
    for (auto& c : cols_) {
        auto [res, _] = c->ThrowRowGroup();
        // std::cout << "! " << res << " " << res->GetSize() << " " << c << std::endl;
        if (is_groupby_ && res->GetSize() > 1) {
            ans.push_back(Do<OFront>(res));
        } else {
            ans.push_back(std::move(res));
        }
    }
    return std::move(ans);
}

std::vector<std::string> TAgregationsEngine::GetNames() {
    std::vector<std::string> ans;
    ans.reserve(cols_.size());
    for (auto& agr : cols_) {
        ans.push_back(agr->GetName());
    }
    for (auto [i, name] : aliases_) {
        ans[i] = name;
    }
    return std::move(ans);
}

} // namespace JFEngine