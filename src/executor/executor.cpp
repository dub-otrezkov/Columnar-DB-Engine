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

    auto [t, err1] = ParseCommand(query);
    if (err1 != EError::NoError) {
        return err1;
    }

    auto& tokens = t->cmds;

    TEngine eng;
    {
        auto [inp, err2] = tokens[1]->MakeWorker();
        if (err2 != EError::NoError) {
            return err2;
        }
        TIoFactory::RegisterTableInput(
            kCurTableInput,
            inp
        );
    }
    {
        TIoFactory::RegisterFileIo(
            kTmp1,
            ETypeFile::kJfFile
        );
        TIoFactory::RegisterFileIo(
            kTmp2,
            ETypeFile::kJfFile
        );
    }

    auto cur_t1 = kTmp1;
    auto cur_t2 = kTmp2;

    for (ui64 i = 2; i < tokens.size(); i++) {
        if (tokens[i]->GetType() == ETokens::kGroup) {
            auto* gt = dynamic_cast<TGroupToken*>(tokens[i]);
            auto* sl = dynamic_cast<TSelectToken*>(tokens[0]);

            if (!sl) {
                return EError::BadCmdErr;
            }

            gt->SetSelects(ParseArgs(sl->GetArgs()));
            sl->SetIsId();
        }
        auto [inp, err] = tokens[i]->MakeWorker();
        if (err != EError::NoError) {
            return err;
        }

        TIoFactory::RegisterTableInput(
            kCurTableInput,
            inp
        );

        std::swap(cur_t1, cur_t2);
    }

    auto [_, err3] = tokens[0]->MakeWorker();
    if (err3 != EError::NoError) {
        return err3;
    }

    return EError::NoError;
}

} // namespace JfEngine
