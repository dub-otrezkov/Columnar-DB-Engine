#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

#include <boost/unordered/unordered_flat_map.hpp>

#include <functional>

namespace JfEngine {

TAoQuery ParseArgs(const std::vector<IToken*>& inp, bool has_group_by) {

    std::vector<std::pair<ui64, std::string>> aliases;

    bool next_alias = false;

    std::vector<std::unique_ptr<IOa>> obs;
    std::vector<ui64> args;
    std::vector<ui64> st;

    std::vector<std::pair<ui64, ui64>> eds;

    EAoEngineType etype = EAoEngineType::kOperator;

    std::string last_col = "";

    for (auto token : inp) {
        if (next_alias) {
            next_alias = false;
            if (token->GetType() == ETokens::kNameToken) {
                aliases.emplace_back(
                    args.size() - 1,
                    static_cast<TNameToken*>(token)->GetName()
                );
                continue;
            }
        }
        switch (token->GetType()) {
            case ETokens::kCloseBracket: {
                st.pop_back();
                break;
            }
            case ETokens::kAs: {
                next_alias = true;
                break;
            }
            case ETokens::kNameToken: {
                auto d = static_cast<TNameToken*>(token)->GetName();

                obs.push_back(std::make_unique<TColumnOp>(d));

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                break;
            }
            case ETokens::kSum: {
                obs.push_back(std::make_unique<TSumAgr>());
                etype = EAoEngineType::kAgregation;

                args.push_back(obs.size() - 1);

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kCount: {
                obs.push_back(std::make_unique<TCountAgr>());
                etype = EAoEngineType::kAgregation;

                args.push_back(obs.size() - 1);

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kAvg: {
                obs.push_back(std::make_unique<TAvgAgr>());
                etype = EAoEngineType::kAgregation;

                args.push_back(obs.size() - 1);

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kMin: {
                obs.push_back(std::make_unique<TMinAgr>());
                etype = EAoEngineType::kAgregation;

                args.push_back(obs.size() - 1);

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kMax: {
                obs.push_back(std::make_unique<TMaxAgr>());
                etype = EAoEngineType::kAgregation;

                args.push_back(obs.size() - 1);

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kDistinct: {
                obs.push_back(std::make_unique<TDistinctOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kExtractMinute: {
                obs.push_back(std::make_unique<TExtractMinuteOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kTruncMinute: {
                obs.push_back(std::make_unique<TTruncMinuteOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kLength: {
                obs.push_back(std::make_unique<TLengthOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kPlus: {
                obs.push_back(std::make_unique<TPlusOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kMinus: {
                obs.push_back(std::make_unique<TMinusOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
        }
    }
    for (auto& p : args) {
        obs[p]->is_final = true;
    }
    std::vector<std::vector<ui64>> gr(obs.size());
    for (const auto& [i, j] : eds) {
        gr[i].push_back(j);
    }
    std::vector<bool> used(gr.size(), false);

    std::vector<ui64> topsort;

    auto dfs = [&](auto dfs, ui64 v) -> void {
        used[v] = true;
        for (const auto& to : gr[v]) {
            if (!used[to]) {
                dfs(dfs, to);
            }
        }
        topsort.push_back(v);
    };

    for (ui64 i = 0; i < gr.size(); i++) {
        if (!used[i] && obs[i]->is_final) {
            dfs(dfs, i);
        }
    }

    std::vector<ui64> inv(topsort.size());
    for (ui64 i = 0; i < topsort.size(); i++) {
        inv[topsort[i]] = i;
    }

    std::vector<std::unique_ptr<IOa>> all(obs.size());

    for (ui64 i = 0; i < all.size(); i++) {
        all[inv[i]] = std::move(obs[i]);
    }

    for (auto& [i, j] : eds) {
        i = inv[i];
        j = inv[j];

        all[i]->AddArg(all[j].get());
    }

    boost::unordered_flat_map<std::string, ui64> alias;
    std::vector<std::unique_ptr<IOa>> fin;
    for (ui64 i = 0; i < all.size(); i++) {
        if (!alias.contains(all[i]->GetName())) {
            alias.emplace(all[i]->GetName(), fin.size());
            fin.push_back(all[i]->Clone());
        }
    }

    for (auto& [i, j] : eds) {
        i = alias.at(all[i]->GetName());
        j = alias.at(all[j]->GetName());

        fin[i]->AddArg(fin[j].get());
    }

    return TAoQuery{
        std::move(eds),
        std::move(fin),
        std::move(aliases),
        etype
    };
}

Expected<ITableInput> TSelectToken::MakeWorker() {
    if (!is_id_) {
        auto args = ParseArgs(args_);

        TIoFactory::GetTableIo(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput).GetShared(),
            std::move(args)
        );

        TEngine eng;

        TIoFactory::RegisterFileIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(kResultData, ETypeFile::kCsvFile);

        eng.Setup(agr);

        eng.WriteDataToCsv(TIoFactory::GetIo(kResultData).GetRes());
        eng.WriteSchemeToCsv(TIoFactory::GetIo(kResultScheme).GetRes());

        return EError::NoError;
    } else {
        TIoFactory::GetTableIo(kCurTableInput).GetShared()->SetupColumnsScheme();

        auto agr = std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput).GetShared()
        );

        TEngine eng;

        TIoFactory::RegisterFileIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(kResultData, ETypeFile::kCsvFile);

        eng.Setup(agr);

        eng.WriteDataToCsv(TIoFactory::GetIo(kResultData).GetRes());
        eng.WriteSchemeToCsv(TIoFactory::GetIo(kResultScheme).GetRes());

        return EError::NoError;
    }
}

} // namespace JfEngine
