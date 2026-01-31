#include "executor.h"

#include "engine/engine.h"
#include "executor/sql_parser/tokenizer.h"

#include <fstream>

namespace JFEngine {

Expected<void> TExecutor::ExecQuery(const std::string& query) {
    auto el = ParseCommand(query);

    if (el.HasError()) {
        return el.GetError();
    }

    auto tokens = el.GetRes();

    std::vector<std::ifstream> s;

    switch (tokens[0]->GetType()) {
    case TTokens::ECreate: {
        if (tokens.size() != 2 || tokens[1]->GetType() != TTokens::EFrom) {
            return MakeError<BadCmdErr>();
        }
        auto [eng, err] = ExecuteNode<TFromToken>(tokens[1]);

        if (err) {
            return err;
        }

        // return tokens[0]->Execute(
        //     [&eng](
        //     TCommands cmd, 
        //     const std::vector<std::shared_ptr<IToken>>& args) -> Expected<void> {
        //         if (args.size() != 1 || args[0]->GetType() != TTokens::TNameToken) {
        //             return MakeError<BadCmdErr>();
        //         }
        //         std::ofstream out(std::dynamic_pointer_cast<TNameToken>(args[0])->GetName() + ".jf");
        //         return eng->WriteTableToJF(out);
        //     }
        // );

        return nullptr;

        break;
    }
    default:
        std::cout << "cant exec" << std::endl;
        break;
    }
}

} // namespace JFEngine