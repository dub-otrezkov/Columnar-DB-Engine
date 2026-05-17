#include "tokenizer.h"

#include "ios_factory/ios_factory.h"
#include "workers/global_agregations/agregator.h"

#include <boost/unordered/unordered_flat_map.hpp>

#include <cassert>
#include <functional>

namespace JfEngine {

TAoQuery ParseArgs(const std::vector<IToken*>& inp, bool has_group_by) {

    std::vector<std::pair<ui64, std::string>> aliases;

    bool next_alias = false;
    bool if_flag = false;

    std::vector<std::unique_ptr<IOa>> obs;
    std::vector<ui64> args;
    std::vector<ui64> st;

    std::vector<std::pair<ui64, ui64>> eds;

    EAoEngineType etype = EAoEngineType::kOperator;

    std::string last_col = "";

    std::vector<IToken*> for_if;

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
        if (if_flag) {
            if (token->GetType() == ETokens::kCloseBracket) {
                auto [if_token, err] = ParseIf(for_if);
                if (err) {
                    throw "??????";
                }
                assert(if_token.size() == 3);
                obs.push_back(std::move(if_token[0]));
                obs.push_back(std::move(if_token[1]));
                obs.push_back(std::move(if_token[2]));
                eds.emplace_back(obs.size() - 1, obs.size() - 3);
                eds.emplace_back(obs.size() - 1, obs.size() - 2);
                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }
                for_if.clear();
                if_flag = false;
                continue;
            } else if (token->GetType() == ETokens::kOpenBracket) {
                continue;
            } else {
                for_if.push_back(std::move(token));
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
            case ETokens::kCountDistinct: {
                obs.push_back(std::make_unique<TCountDistinctAgr>());
                etype = EAoEngineType::kAgregation;

                args.push_back(obs.size() - 1);

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
            case ETokens::kRegexpReplace: {
                obs.push_back(std::make_unique<TRegexpReplaceOp>());

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
            case ETokens::kConstInt: {
                obs.push_back(std::make_unique<TConstIntOp>());

                if (!st.empty()) {
                    eds.emplace_back(st.back(), obs.size() - 1);
                } else {
                    args.push_back(obs.size() - 1);
                }

                st.push_back(obs.size() - 1);

                break;
            }
            case ETokens::kIf: {
                if_flag = true;
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

    // Cache names before moving — needed for the edge remap below since
    // the moved-from unique_ptrs become null.
    std::vector<std::string> all_names(all.size());
    for (ui64 i = 0; i < all.size(); i++) {
        all_names[i] = all[i]->GetName();
    }

    boost::unordered_flat_map<std::string, ui64> alias;
    std::vector<std::unique_ptr<IOa>> fin;
    for (ui64 i = 0; i < all.size(); i++) {
        if (!alias.contains(all_names[i])) {
            alias.emplace(all_names[i], fin.size());
            all[i]->ClearArgs();
            fin.push_back(std::move(all[i]));
        }
    }

    for (auto& [i, j] : eds) {
        i = alias.at(all_names[i]);
        j = alias.at(all_names[j]);

        fin[i]->AddArg(fin[j].get());
    }

    return TAoQuery{
        std::move(eds),
        std::move(fin),
        std::move(aliases),
        etype
    };
}

Expected<TTableInputPtr> TSelectToken::MakeWorker() {
    if (!is_id_) {
        auto args = ParseArgs(args_);

        auto agr = std::make_shared<TAgregator>(
            TIoFactory::GetTableIo(kCurTableInput),
            std::move(args)
        );

        TEngine eng;

        TIoFactory::RegisterFileIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(kResultData, ETypeFile::kCsvFile);

        eng.Setup(agr);

        eng.WriteDataToCsv(*TIoFactory::GetIo(kResultData));
        eng.WriteSchemeToCsv(*TIoFactory::GetIo(kResultScheme));

        return EError::NoError;
    } else {
        TEngine eng;

        TIoFactory::RegisterFileIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterFileIo(kResultData, ETypeFile::kCsvFile);

        eng.Setup(TIoFactory::GetTableIo(kCurTableInput));

        eng.WriteDataToCsv(*TIoFactory::GetIo(kResultData));
        eng.WriteSchemeToCsv(*TIoFactory::GetIo(kResultScheme));

        return EError::NoError;
    }
}

} // namespace JfEngine
