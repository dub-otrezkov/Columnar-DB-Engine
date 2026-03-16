#pragma once

#include "workers/agregations_engine/engine.h"
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
    kMax,
    kMin,
    kOpenBracket,
    kCloseBracket,
    kComa, // misc (
    kWhere,
    kBy, // misc (
    kGroup,
    kOrder,
    kLimit
};

static const std::unordered_map<std::string, ETokens> cmds = {
    {"FROM", ETokens::kFrom},
    {"WHERE", ETokens::kWhere},
    {"GROUP", ETokens::kGroup},
    {"ORDER", ETokens::kOrder},
    {"LIMIT", ETokens::kLimit},
    {"SELECT", ETokens::kSelect},
    {"CREATE", ETokens::kCreate},
};

static const std::unordered_map<std::string, ETokens> operators = {
    {"SUM", ETokens::kSum},
    {"COUNT", ETokens::kCount},
    {"AVG", ETokens::kAvg},
    {"MIN", ETokens::kMin},
    {"MAX", ETokens::kMax},
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

// commands tokens

class TSelectToken : public ICommand {
public:

    ETokens GetType() const override;
    
    TGlobalAgregationQuery ParseArgs();
    Expected<ITableInput> Exec() override;

    void SetIsId();

private:
    bool is_id_ = false;
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

class TLimitToken : public ICommand {
public:

    ETokens GetType() const override;

    ui64 GetLimit();
    
    Expected<ITableInput> Exec() override;
};

class TOrderToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> Exec() override;

    std::shared_ptr<TLimitToken> limit_;

};

class TWhereToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
};

class TGroupToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> Exec() override;
    void SetSelects(TGlobalAgregationQuery s);

    std::shared_ptr<TLimitToken> limit_;

private:
    TGlobalAgregationQuery selects_;
};

// operator cmds tokens

class TSumToken : public IOperatorCommand {
public:
    ETokens GetType() const override;
};

class TMinToken : public IOperatorCommand {
public:
    ETokens GetType() const override;
};

class TMaxToken : public IOperatorCommand {
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

// misc

class TAsToken : public IToken {
public:
    ETokens GetType() const override;
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
