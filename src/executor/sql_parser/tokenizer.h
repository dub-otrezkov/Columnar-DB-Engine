#pragma once

#include "engine/engine.h"
#include "utils/errors/errors.h"
#include "errors.h"

#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace JFEngine {

enum class TTokens {
    ENameToken,
    ECreate,
    EFrom,
    EAs,
};

static const std::unordered_map<std::string, TTokens> cmds = {
    {"FROM", TTokens::EFrom},
    {"AS", TTokens::EAs},
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

    void AddArg(std::shared_ptr<IToken> arg) {
        args_.push_back(arg);
    }
protected:
    std::vector<std::shared_ptr<IToken>> args_;
};

template <typename T>
class ICommandExec {
public:
    virtual ~ICommandExec() = default;

    using TReturn = Expected<T>;

    virtual TReturn Exec() = 0;

};

class TCreateToken : public ICommand, public ICommandExec<void> {
public:
    TReturn Exec();

    TTokens GetType() const override;

};

class TFromToken : public ICommand, public ICommandExec<TEngine> {
public:
    TReturn Exec();

    TTokens GetType() const override;

private:
    std::vector<std::ifstream> s;
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
T::TReturn ExecuteNode(std::shared_ptr<ICommand> node) {
    auto d = std::dynamic_pointer_cast<T>(node);

    if (!d) {
        return MakeError<BadCmdErr>();
    }

    return d->Exec();
}

} // namespace JFEngine
