#pragma once

#include "agregations.h"
#include "operators.h"

namespace JfEngine {

struct TAgregationQuery {
    std::vector<std::unique_ptr<IOa>> cols;
    std::vector<std::pair<ui64, ui64>> edges;

    TAgregationQuery() {}

    TAgregationQuery(
        std::vector<std::unique_ptr<IOa>> cols_,
        std::vector<std::pair<ui64, ui64>> edges_
    ) :
        cols(std::move(cols_)),
        edges(std::move(edges_))
    {}

    TAgregationQuery(const TAgregationQuery&) = delete;
    TAgregationQuery& operator=(const TAgregationQuery&) = delete;
    TAgregationQuery(TAgregationQuery&&) = default;
    TAgregationQuery& operator=(TAgregationQuery&&) = default;
};

struct TOperatorQuery {
    std::vector<std::unique_ptr<IOa>> cols;
    std::vector<std::pair<ui64, ui64>> edges;

    TOperatorQuery() {}

    TOperatorQuery(
        std::vector<std::unique_ptr<IOa>> cols_,
        std::vector<std::pair<ui64, ui64>> edges_
    ) :
        cols(std::move(cols_)),
        edges(std::move(edges_))
    {}

    TOperatorQuery(const TOperatorQuery&) = delete;
    TOperatorQuery& operator=(const TOperatorQuery&) = delete;
    TOperatorQuery(TOperatorQuery&&) = default;
    TOperatorQuery& operator=(TOperatorQuery&&) = default;
};

enum class EAoEngineType {
    kAgregation,
    kOperator
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

    TAoQuery() = default;
    TAoQuery(const TAoQuery&) = delete;
    TAoQuery& operator=(const TAoQuery&) = delete;

    TAoQuery(TAoQuery&&) = default;
    TAoQuery& operator=(TAoQuery&&) = default;

    TAoQuery Clone();
};

class IAoEngine {
public:
    virtual ~IAoEngine() = default;

    IAoEngine() = default;
    IAoEngine(std::vector<std::pair<ui64, std::string>> aliases);

    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual std::vector<TColumnPtr> ThrowRowGroup() = 0;
    virtual std::shared_ptr<IAoEngine> Clone() = 0;

    virtual EAoEngineType GetType() const = 0;

    virtual std::vector<std::string>& GetNames();

protected:
    std::vector<std::string> names;
};

std::shared_ptr<IAoEngine> MakeAoEngine(TAoQuery);

class TOperatorEngine : public IAoEngine {
public:
    TOperatorEngine() = default;
    TOperatorEngine(
        TOperatorQuery qry,
        std::vector<std::pair<ui64, std::string>> aliases = {}
    );

    EAoEngineType GetType() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    std::vector<TColumnPtr> ThrowRowGroup() override;
    std::vector<std::string>& GetNames() override;
    std::shared_ptr<IAoEngine> Clone() override;

private:
    TOperatorQuery q_;
};

class TAgregationEngine : public IAoEngine {
public:
    TAgregationEngine() = default;
    TAgregationEngine(
        TAgregationQuery q,
        std::vector<std::pair<ui64, std::string>> aliases = {}
    );

    EAoEngineType GetType() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    std::vector<TColumnPtr> ThrowRowGroup() override;
    std::vector<std::string>& GetNames() override;
    std::shared_ptr<IAoEngine> Clone() override;

private:
    TAgregationQuery q_;
};

} // namespace JfEngine
