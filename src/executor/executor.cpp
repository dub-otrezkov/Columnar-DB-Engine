#include "executor.h"

#include "ios_factory/ios_factory.h"
#include "engine/engine.h"
#include "executor/sql_parser/tokenizer.h"

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

    auto tokens = *t;

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
            auto gt = std::dynamic_pointer_cast<TGroupToken>(tokens[i]);
            auto sl = std::dynamic_pointer_cast<TSelectToken>(tokens[0]);

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

        // if (auto d = std::dynamic_pointer_cast<std::stringstream>(TIoFactory::GetIo(cur_t2).GetShared())) {
        //     d->clear();
        //     d->seekg(0, std::ios::beg);
        //     d->seekp(0, std::ios::beg);
        //     d->str("");
        // } else if (auto d = std::dynamic_pointer_cast<std::fstream>(TIoFactory::GetIo(cur_t2).GetShared())) {
        //     d->close();
        //     d->open(cur_t2 + ".jf", std::ios::out | std::ios::in | std::ios::trunc);
        // }
        // // TIoFactory::GetIo(cur_t2).GetShared()->clear();
        // // TIoFactory::GetIo(cur_t2).GetShared()->seekg(0, std::ios::beg);
        // // TIoFactory::GetIo(cur_t2).GetShared()->seekp(0, std::ios::beg);

        std::swap(cur_t1, cur_t2);
    }

    auto [_, err3] = tokens[0]->MakeWorker();
    if (err3 != EError::NoError) {
        return err3;
    }

    return EError::NoError;
}

} // namespace JfEngine