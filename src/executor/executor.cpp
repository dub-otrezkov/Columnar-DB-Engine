#include "executor.h"

#include "engine/engine.h"
#include "executor/sql_parser/tokenizer.h"
#include "ios_factory/ios_factory.h"

#include <fstream>

namespace JfEngine {

static const std::string kTmp1 = "tmp1";
static const std::string kTmp2 = "tmp2";

TExecutor::~TExecutor() {
    TIoFactory::Clear();
}

Expected<void> TExecutor::ExecQuery(const std::string& query) {
    TIoFactory::Clear();
    TMemoryArena::Instance().Reset();

    stats_.Reset();
    TQueryStats::instance = &stats_;

    auto [t, err1] = ParseCommand(query);
    if (err1 != EError::NoError) {
        TQueryStats::instance = nullptr;
        return err1;
    }

    auto& tokens = t.cmds;

    TEngine eng;
    {
        auto inp = tokens[1]->MakeWorker();
        if (inp.HasError()) {
            TQueryStats::instance = nullptr;
            return inp.GetError();
        }
        TIoFactory::RegisterTableInput(kCurTableInput, inp.GetRes());
    }

    for (ui64 i = 2; i < tokens.size(); i++) {
        if (tokens[i]->GetType() == ETokens::kGroup) {
            auto* gt = dynamic_cast<TGroupToken*>(tokens[i]);
            auto* sl = dynamic_cast<TSelectToken*>(tokens[0]);

            if (!sl) {
                TQueryStats::instance = nullptr;
                return EError::BadCmdErr;
            }

            gt->SetSelects(ParseArgs(sl->GetArgs()));
            sl->SetIsId();
        }
        auto inp = tokens[i]->MakeWorker();
        if (inp.HasError()) {
            TQueryStats::instance = nullptr;
            return inp.GetError();
        }

        TIoFactory::RegisterTableInput(kCurTableInput, inp.GetRes());
    }

    auto [_, err3] = tokens[0]->MakeWorker();
    TQueryStats::instance = nullptr;
    if (err3 != EError::NoError) {
        return err3;
    }

    stats_.Print(std::cout);

    return EError::NoError;
}

} // namespace JfEngine
