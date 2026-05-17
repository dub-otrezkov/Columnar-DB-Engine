#pragma once

#include "agregations.h"
#include "operators.h"

namespace JfEngine {

enum class EAoEngineType {
    kAgregation,
    kOperator
};

class IAoEngine {
public:
    virtual ~IAoEngine() = default;

    IAoEngine() = default;
    IAoEngine(std::vector<std::unique_ptr<IOa>> cols, std::vector<std::pair<ui64, std::string>> aliases);

    virtual Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* i = nullptr);
    virtual std::vector<TColumnPtr> ThrowRowGroup();

    virtual EAoEngineType GetType() const = 0;

    virtual std::vector<std::string>& GetNames();
protected:
    std::vector<std::unique_ptr<IOa>> cols_;
    std::vector<std::string> names_;
};

struct TAoQuery {
    std::vector<std::pair<ui64, ui64>> edges;
    std::vector<std::unique_ptr<IOa>> args;
    std::vector<std::pair<ui64, std::string>> aliases;
    EAoEngineType tp;

    TAoQuery(
        std::vector<std::pair<ui64, ui64>> e,
        std::vector<std::unique_ptr<IOa>> a,
        std::vector<std::pair<ui64, std::string>> b,
        EAoEngineType c
    ) :
        edges(std::move(e)),
        args(std::move(a)),
        aliases(std::move(b)),
        tp(c)
    {}

    TAoQuery(
        std::vector<std::unique_ptr<IOa>> a,
        std::vector<std::pair<ui64, std::string>> b,
        EAoEngineType c
    ) :
        args(std::move(a)),
        aliases(std::move(b)),
        tp(c)
    {}

    TAoQuery() = default;
    TAoQuery(const TAoQuery&) = delete;
    TAoQuery& operator=(const TAoQuery&) = delete;

    TAoQuery(TAoQuery&&) = default;
    TAoQuery& operator=(TAoQuery&&) = default;
};

std::shared_ptr<IAoEngine> MakeAoEngine(TAoQuery);

class TOperatorEngine : public IAoEngine {
public:
    TOperatorEngine() = default;
    TOperatorEngine(
        std::vector<std::unique_ptr<IOa>> q,
        std::vector<std::pair<ui64, std::string>> aliases
    );

    EAoEngineType GetType() const override;
};

class TAgregationEngine : public IAoEngine {
public:
    TAgregationEngine() = default;
    TAgregationEngine(
        std::vector<std::unique_ptr<IOa>> q,
        std::vector<std::pair<ui64, std::string>> aliases
    );

    EAoEngineType GetType() const override;
};

} // namespace JfEngine
