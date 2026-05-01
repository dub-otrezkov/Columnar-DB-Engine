#include "agregator.h"

#include "columns/operators/operators.h"
#include "utils/perf_stats/perf_stats.h"

namespace JfEngine {

TAgregator::TAgregator(TTableInputPtr jf_in, TAoQuery query) :
    jf_in_(std::move(jf_in)),
    eng_(MakeAoEngine(std::move(query)))
{}

TAgregator::TAgregator(TTableInputPtr jf_in) : jf_in_(std::move(jf_in)), is_all_(true) {
}

std::vector<TRowScheme>& TAgregator::GetScheme() {
    if (is_all_) {
        return jf_in_->GetScheme();
    } else {
        return scheme_;
    }
}

Expected<void> TAgregator::SetupColumnsScheme() {
    if (!scheme_.empty()) {
        return nullptr;
    }
    auto err = jf_in_->SetupColumnsScheme();
    if (is_all_) {
        return err;
    }
    if (err.HasError()) {
        return err.GetError();
    }
    auto names = eng_->GetNames();
    cols_cnt_ = names.size();
    for (ui64 i = 0; i < cols_cnt_; i++) {
        scheme_.emplace_back(names[i], EColumn::kUnitialized);
    }
    return nullptr;
}

Expected<std::vector<TColumnPtr>> TAgregator::LoadRowGroup() {
    if (!is_all_) {
        bool is_eof = false;
        std::vector<TColumnPtr> ans;
        const char* eng_name = eng_->GetType() == EAoEngineType::kAgregation ? "AgregEngine" : "OperEngine";

        if (eng_->GetType() == EAoEngineType::kAgregation) {
            bool run = true;
            for (; run; jf_in_->MoveCursor()) {
                TAoEngineTimer t(eng_name);
                auto err = eng_->ConsumeRowGroup(jf_in_.get());
                if (err.HasError()) {
                    if (err.GetError() == EError::EofErr) {
                        run = false;
                    } else {
                        return err.GetError();
                    }
                }
            }
            {
                TAoEngineTimer t(eng_name);
                ans = eng_->ThrowRowGroup();
            }
            is_eof = true;
        } else {
            {
                TAoEngineTimer t(eng_name);
                auto err = eng_->ConsumeRowGroup(jf_in_.get());
                if (err.HasError()) {
                    if (err.GetError() != EError::EofErr) {
                        return err.GetError();
                    } else {
                        is_eof = true;
                    }
                }
            }
            {
                TAoEngineTimer t(eng_name);
                ans = eng_->ThrowRowGroup();
            }
        }

        for (ui64 i = 0; i < ans.size(); i++) {
            scheme_[i].type_ = ans[i]->GetType();
        }

        assert(ans.size() == GetScheme().size());
        
        return {std::move(ans), (is_eof ? EError::EofErr : EError::NoError)};
    } else {
        assert(jf_in_->GetScheme().size() == GetScheme().size());
        return jf_in_->ReadRowGroup();
    }
}

void TAgregator::MoveCursor() {
    current_rg_.reset();
    current_rg_err_ = EError::NoError;
    jf_in_->MoveCursor();
}

} // namespace JfEngine