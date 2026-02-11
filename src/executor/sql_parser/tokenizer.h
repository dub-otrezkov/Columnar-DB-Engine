#pragma once

#include "engine/engine.h"
#include "utils/errors/errors_templates.h"

#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace JFEngine {

enum class ETokens {
    ENameToken,
    EFrom,
    ECreate,
    ESelect,
    EAs,
    ESum,
};

static const std::unordered_map<std::string, ETokens> cmds = {
    {"FROM", ETokens::EFrom},
    {"SELECT", ETokens::ESelect},
    {"CREATE", ETokens::ECreate},
};

static const std::unordered_map<std::string, ETokens> operators = {
    {"SUM", ETokens::ESum},
};

class IToken {
public:
    virtual ~IToken() = default;

    virtual ETokens GetType() const = 0;
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

class IOperatorCommand : public ICommand {
public:
    virtual Expected<IColumn> Run() = 0;
};

class TSelectToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TCreateToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TFromToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TNameToken : public IToken {
public:
    TNameToken(std::string name);

    ETokens GetType() const override;

    std::string GetName() const;
private:
    std::string name_;
};

class OpenBracketToken : public IToken {
};

class CloseBracketToken : public IToken {
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
