#pragma once

#include "engine/engine.h"
#include "utils/errors/errors_templates.h"
#include "workers/ao_engine/engine.h"

#include <memory>
#include <sstream>
#include <string>
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

    inline ETokens GetType() const override { return ETokens::kSelect; }

    inline std::vector<std::shared_ptr<IToken>> GetArgs() { return args_; }
    Expected<ITableInput> MakeWorker() override;

    inline void SetIsId() { is_id_ = true; }

private:
    bool is_id_ = false;
};

class TCreateToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kCreate; }

    Expected<ITableInput> MakeWorker() override;
};

struct TFromToken : public ICommand {
    inline TFromToken(std::string query) : query_(std::move(query)) {}

    inline ETokens GetType() const override { return ETokens::kFrom; }

    Expected<ITableInput> MakeWorker() override;

    std::string query_;
};

class TLimitToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kLimit; }

    ui64 GetLimit() const;

    Expected<ITableInput> MakeWorker() override;
};

class TOrderToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kOrder; }

    Expected<ITableInput> MakeWorker() override;

    std::shared_ptr<TLimitToken> limit_;

};

class TWhereToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kWhere; }

    Expected<ITableInput> MakeWorker() override;
};

class TGroupToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kGroup; }

    Expected<ITableInput> MakeWorker() override;
    inline void SetSelects(TAoQuery s) { selects_ = std::move(s); }

    std::shared_ptr<TLimitToken> limit_;

private:
    TAoQuery selects_;
};

// operator cmds tokens

class TSumToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kSum; }
};

class TMinToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kMin; }
};

class TMaxToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kMax; }
};

class TCountToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kCount; }
};

class TDistinctToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kDistinct; }
};

class TAvgToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kAvg; }
};

class TLengthToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kLength; }
};

class TPlusToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kPlus; }
};

class TMinusToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kMinus; }
};

class TNameToken : public IToken {
public:
    inline TNameToken(std::string name) : name_(std::move(name)) {}

    inline ETokens GetType() const override { return ETokens::kNameToken; }

    inline std::string GetName() const { return name_; }
private:
    std::string name_;
};

// misc

class TAsToken : public IToken {
public:
    inline ETokens GetType() const override { return ETokens::kAs; }
};

class TAndToken : public IToken {
public:
    inline ETokens GetType() const override { return ETokens::kAnd; }
};

class TOpenBracketToken : public IToken {
public:
    inline ETokens GetType() const override { return ETokens::kOpenBracket; }
};

class TComaToken : public IToken {
public:
    inline ETokens GetType() const override { return ETokens::kComa; }
};

class TCloseBracketToken : public IToken {
public:
    inline ETokens GetType() const override { return ETokens::kCloseBracket; }
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
