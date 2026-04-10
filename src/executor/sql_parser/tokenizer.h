#pragma once

#include "workers/ao_engine/engine.h"
#include "engine/engine.h"
#include "utils/errors/errors_templates.h"

#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace JfEngine {

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
    kAnd,
    kDistinct,
    kLength,
    kPlus,
    kMinus,
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
    {"FROM",   ETokens::kFrom},
    {"WHERE",  ETokens::kWhere},
    {"GROUP",  ETokens::kGroup},
    {"ORDER",  ETokens::kOrder},
    {"LIMIT",  ETokens::kLimit},
    {"SELECT", ETokens::kSelect},
    {"CREATE", ETokens::kCreate},
};

static const std::unordered_map<std::string, ETokens> agregations = {
    {"SUM",      ETokens::kSum},
    {"COUNT",    ETokens::kCount},
    {"AVG",      ETokens::kAvg},
    {"MIN",      ETokens::kMin},
    {"MAX",      ETokens::kMax},
    {"DISTINCT", ETokens::kDistinct},
};

static const std::unordered_map<std::string, ETokens> operators = {
    {"+",        ETokens::kPlus},
    {"-",        ETokens::kMinus},
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

    virtual Expected<ITableInput> MakeWorker() = 0;

    void AddArg(std::shared_ptr<IToken> arg) {
        args_.push_back(arg);
    }
protected:
    std::vector<std::shared_ptr<IToken>> args_;
};

class IoperatorCommand : public ICommand {
public:
    Expected<ITableInput> MakeWorker() override;
};

// commands tokens

class TSelectToken : public ICommand {
public:

    ETokens GetType() const override;
    
    std::vector<std::shared_ptr<IToken>> GetArgs();
    Expected<ITableInput> MakeWorker() override;

    void SetIsId();

private:
    bool is_id_ = false;
};

class TCreateToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> MakeWorker() override;
};

struct TFromToken : public ICommand {
    TFromToken(std::string query);

    ETokens GetType() const override;
    
    Expected<ITableInput> MakeWorker() override;

    std::string query_;
};

class TLimitToken : public ICommand {
public:

    ETokens GetType() const override;

    ui64 GetLimit();
    
    Expected<ITableInput> MakeWorker() override;
};

class TOrderToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> MakeWorker() override;

    std::shared_ptr<TLimitToken> limit_;

};

class TWhereToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> MakeWorker() override;
};

class TGroupToken : public ICommand {
public:

    ETokens GetType() const override;
    
    Expected<ITableInput> MakeWorker() override;
    void SetSelects(TAoQuery s);

    std::shared_ptr<TLimitToken> limit_;

private:
    TAoQuery selects_;
};

// operator cmds tokens

class TSumToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TMinToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TMaxToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TCountToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TDistinctToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TAvgToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TLengthToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TPlusToken : public IoperatorCommand {
public:
    ETokens GetType() const override;
};

class TMinusToken : public IoperatorCommand {
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

class TAndToken : public IToken {
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
    std::string query_;
    std::stringstream ss;
};

TAoQuery ParseArgs(std::vector<std::shared_ptr<IToken>> inp);

Expected<std::vector<std::shared_ptr<ICommand>>> ParseCommand(const std::string& cmd);

} // namespace JfEngine
