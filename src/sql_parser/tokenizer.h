#pragma once

#include <string>
#include <unordered_map>

namespace JFEngine {

enum class TCommands {
    ESelect,
    EFrom,
    EAs,
};

static const std::unordered_map<std::string, TCommands> cmds = {
    {"SELECT", ESelect},
    {"FROM", EFrom},
    {"AS", EAs},
};

class TCommandToken {
public:
    TCommandToken(TCommands tp);
private:
    TCommands type_;
};

class TColumnToken {
public:
    TColumnToken(std::string name);
private:
    std::string name;
};

} // namespace JFEngine
