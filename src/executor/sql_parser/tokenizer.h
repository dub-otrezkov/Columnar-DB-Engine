#pragma once

#include "engine/engine.h"
#include "utils/errors/errors_templates.h"

#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace JFEngine {

enum class TTokens {
    ENameToken,
    EFrom,
    ECreate,
    ESelect,
    EAs,
};

static const std::unordered_map<std::string, TTokens> cmds = {
    {"FROM", TTokens::EFrom},
    {"SELECT", TTokens::ESelect},
    {"CREATE", TTokens::ECreate},
};

class IToken {
public:
    virtual ~IToken() = default;

    virtual TTokens GetType() const = 0;
private:
};

class ICommand : public IToken {
public:
    virtual ~ICommand() = default;

    virtual Expected<ITableInput> Exec() = 0;

    void AddArg(std::shared_ptr<IToken> arg) {
        args_.push_back(arg);
    }
protected:
    std::vector<std::shared_ptr<IToken>> args_;
};

class TSelectToken : public ICommand {
public:

    TTokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TCreateToken : public ICommand {
public:

    TTokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TFromToken : public ICommand {
public:

    TTokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TNameToken : public IToken {
public:
    TNameToken(std::string name);

    TTokens GetType() const override;

    std::string GetName() const;
private:
    std::string name_;
};

class Tokenizer {
public:
    Tokenizer(const std::string& data);

    Expected<IToken> GetNext();
private:
    std::stringstream ss;
};

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd);

template<typename T>
Expected<TEngine> ExecuteNode(std::shared_ptr<ICommand> node) {
    auto d = std::dynamic_pointer_cast<T>(node);

    if (!d) {
        return MakeError<BadCmdErr>();
    }

    return d->Exec();
}

} // namespace JFEngine
