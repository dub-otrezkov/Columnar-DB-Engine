#include "executor.h"

#include "ios_factory/ios_factory.h"
#include "engine/engine.h"
#include "executor/sql_parser/tokenizer.h"

#include <fstream>

namespace JFEngine {

static const std::string kTmp1 = "tmp1";
static const std::string kTmp2 = "tmp2";

Expected<void> TExecutor::ExecQuery(const std::string& query) {
    auto [t, err1] = ParseCommand(query);
    if (err1 != EError::NoError) {
        return err1;
    }

    auto tokens = *t;

    TEngine eng;
    {
        auto [inp, err2] = tokens[1]->Exec();
        if (err2 != EError::NoError) {
            return err2;
        }
        TIOFactory::RegisterTableInput(
            kCurTableInput,
            inp
        );
    }
    {
        TIOFactory::RegisterFileIO(
            kTmp1,
            ETypeFile::kJFFile
        );
        TIOFactory::RegisterFileIO(
            kTmp2,
            ETypeFile::kJFFile
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
        auto [inp, err] = tokens[i]->Exec();
        if (err != EError::NoError) {
            return err;
        }
        err = eng.Setup(inp).GetError();
        if (err != EError::NoError) {
            return err;
        }

        eng.WriteTableToJF(*TIOFactory::GetIO(cur_t1).GetShared());

        TIOFactory::RegisterTableInput(
            kCurTableInput,
            std::make_shared<TJFTableInput>(
                TIOFactory::GetIO(cur_t1).GetShared()
            )
        );

        if (auto d = std::dynamic_pointer_cast<std::stringstream>(TIOFactory::GetIO(cur_t2).GetShared())) {
            d->clear();
            d->seekg(0, std::ios::beg);
            d->seekp(0, std::ios::beg);
            d->str("");
        } else if (auto d = std::dynamic_pointer_cast<std::fstream>(TIOFactory::GetIO(cur_t2).GetShared())) {
            d->close();
            d->open(cur_t2 + ".jf", std::ios::out | std::ios::in | std::ios::trunc);
        }
        // TIOFactory::GetIO(cur_t2).GetShared()->clear();
        // TIOFactory::GetIO(cur_t2).GetShared()->seekg(0, std::ios::beg);
        // TIOFactory::GetIO(cur_t2).GetShared()->seekp(0, std::ios::beg);

        std::swap(cur_t1, cur_t2);
    }
    
    auto [_, err3] = tokens[0]->Exec();
    if (err3 != EError::NoError) {
        return err3;
    }

    // eng.Setup(fin);

    return nullptr;
}

} // namespace JFEngine