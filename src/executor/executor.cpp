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
    if (err1) {
        return err1;
    }

    auto tokens = *t;

    TEngine eng;
    {
        auto [inp, err2] = tokens[1]->Exec();
        if (err2) {
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
            TFileType::EJFFile
        );
        TIOFactory::RegisterFileIO(
            kTmp2,
            TFileType::EJFFile
        );
    }

    auto cur_t1 = kTmp1;
    auto cur_t2 = kTmp2;

    for (ui64 i = 2; i < tokens.size(); i++) {
        auto [inp, err] = tokens[i]->Exec();
        if (err) {
            return err;
        }
        err = eng.Setup(inp).GetError();
        if (err) {
            return err;
        }

        eng.WriteTableToJF(*TIOFactory::GetIO(cur_t1).GetShared());

        TIOFactory::RegisterTableInput(
            kCurTableInput,
            std::make_shared<TJFTableInput>(
                TIOFactory::GetIO(cur_t1).GetShared()
            )
        );
        std::swap(cur_t1, cur_t2);
    }

    
    auto [_, err3] = tokens[0]->Exec();
    if (err3) {
        return err3;
    }

    return nullptr;
}

} // namespace JFEngine