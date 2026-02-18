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
    kNameToken,
    kFrom,
    kCreate,
    kSelect,
    kAs,
    kSum,
    kCount,
    kAvg,
    kOpenBracket,
    kCloseBracket,
    kComa,
};

static const std::unordered_map<std::string, ETokens> cmds = {
    {"FROM", ETokens::kFrom},
    {"SELECT", ETokens::kSelect},
    {"CREATE", ETokens::kCreate},
};

static const std::unordered_map<std::string, ETokens> operators = {
    {"SUM", ETokens::kSum},
    {"COUNT", ETokens::kCount},
    {"AVG", ETokens::kAvg},
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
    Expected<ITableInput> Exec() override;
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

class TSumToken : public IOperatorCommand {
public:
    ETokens GetType() const override;
};

class TCountToken : public IOperatorCommand {
public:
    ETokens GetType() const override;
};

class TAvgToken : public IOperatorCommand {
public:
    ETokens GetType() const override;
};

class TNameToken : public IToken {
public:
    TNameToken(std::string name);

    ETokens GetType() const override;

    std::string GetName() const;
private:
    std::string name_;
};

class TOpenBracketToken : public IToken {
public:
    ETokens GetType() const override;
};

class TComaToken : public IToken {
public:
    ETokens GetType() const override;
};

class TCloseBracketToken : public IToken {
public:
    ETokens GetType() const override;
};

class TTokenizer {
public:
    TTokenizer(const std::string& data);

    Expected<IToken> GetNext();
private:
    std::stringstream ss;
};

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd);

} // namespace JFEngine
