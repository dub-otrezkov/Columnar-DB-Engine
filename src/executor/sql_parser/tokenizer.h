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
    kExtractMinute,
    kTruncMinute,
    kLength,
    kConstInt,
    kPlus,
    kMinus,
    kOpenBracket,
    kCloseBracket,
    kComa, // misc (
    kWhere,
    kBy, // misc (
    kGroup,
    kOrder,
    kLimit,
    kOffset
};

static const std::unordered_map<std::string, ETokens> cmds = {
    {"FROM",   ETokens::kFrom},
    {"WHERE",  ETokens::kWhere},
    {"GROUP",  ETokens::kGroup},
    {"ORDER",  ETokens::kOrder},
    {"LIMIT",  ETokens::kLimit},
    {"OFFSET", ETokens::kOffset},
    {"SELECT", ETokens::kSelect},
    {"CREATE", ETokens::kCreate},
};

static const std::unordered_map<std::string, ETokens> agregations = {
    {"SUM",      ETokens::kSum},
    {"COUNT",    ETokens::kCount},
    {"AVG",      ETokens::kAvg},
    {"MIN",      ETokens::kMin},
    {"MAX",      ETokens::kMax},
};

static const std::unordered_map<std::string, ETokens> operators = {
    {"+",        ETokens::kPlus},
    {"-",        ETokens::kMinus},
    {"DISTINCT", ETokens::kDistinct},
    {"EXTRACT_MINUTE", ETokens::kExtractMinute},
    {"TRUNC_MINUTE", ETokens::kTruncMinute},
    {"CONST_INT", ETokens::kConstInt},
};

struct IToken {
    virtual ~IToken() = default;

    virtual ETokens GetType() const = 0;
};

struct ICommand : public IToken {
    virtual ~ICommand() = default;

    virtual Expected<TTableInputPtr> MakeWorker() = 0;

    void AddArg(IToken* arg) {
        args_.push_back(arg);
    }

    std::vector<IToken*> args_;
};

struct IoperatorCommand : public ICommand {
    Expected<TTableInputPtr> MakeWorker() override;
};

// commands tokens

struct TSelectToken : public ICommand {
    inline ETokens GetType() const override { return ETokens::kSelect; }

    inline const std::vector<IToken*>& GetArgs() const { return args_; }
    Expected<TTableInputPtr> MakeWorker() override;

    inline void SetIsId() { is_id_ = true; }

    bool is_id_ = false;
};

struct TCreateToken : public ICommand {
    inline ETokens GetType() const override { return ETokens::kCreate; }

    Expected<TTableInputPtr> MakeWorker() override;
};

struct TFromToken : public ICommand {
    inline TFromToken(std::string query) : query_(std::move(query)) {}

    inline ETokens GetType() const override { return ETokens::kFrom; }

    Expected<TTableInputPtr> MakeWorker() override;

    std::string query_;
};

class TLimitToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kLimit; }

    ui64 GetLimit() const;

    Expected<TTableInputPtr> MakeWorker() override;
};

class TOffsetToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kOffset; }

    ui64 GetOffset() const;

    Expected<TTableInputPtr> MakeWorker() override;
};

struct TOrderToken : public ICommand {

    inline ETokens GetType() const override { return ETokens::kOrder; }

    Expected<TTableInputPtr> MakeWorker() override;

    TLimitToken* limit_ = nullptr;
    TOffsetToken* offset_ = nullptr;

};

class TWhereToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kWhere; }

    Expected<TTableInputPtr> MakeWorker() override;
};

class TGroupToken : public ICommand {
public:

    inline ETokens GetType() const override { return ETokens::kGroup; }

    Expected<TTableInputPtr> MakeWorker() override;
    inline void SetSelects(TAoQuery s) { selects_ = std::move(s); }

    TLimitToken* limit_ = nullptr;

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

struct TExtractMinuteToken : public IoperatorCommand {
    inline ETokens GetType() const override { return ETokens::kExtractMinute; }
};

struct TTruncMinuteToken : public IoperatorCommand {
    inline ETokens GetType() const override { return ETokens::kTruncMinute; }
};

class TAvgToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kAvg; }
};

class TLengthToken : public IoperatorCommand {
public:
    inline ETokens GetType() const override { return ETokens::kLength; }
};

struct TConstIntToken : public IoperatorCommand {
    inline ETokens GetType() const override { return ETokens::kConstInt; }
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

    Expected<std::unique_ptr<IToken>> GetNext();
private:
    std::string query_;
    std::stringstream ss;
};

struct TParsedCommand {
    std::vector<std::unique_ptr<IToken>> all;
    std::vector<ICommand*> cmds;
};

TAoQuery ParseArgs(const std::vector<IToken*>& inp, bool has_group_by = false);

Expected<TParsedCommand> ParseCommand(const std::string& cmd);

} // namespace JfEngine
