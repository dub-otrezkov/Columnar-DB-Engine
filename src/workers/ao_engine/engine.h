#pragma once

#include "agregations.h"
#include "operators.h"

namespace JfEngine {

struct TAgregationQuery {
    std::vector<std::shared_ptr<IOa>> cols;

    TAgregationQuery Clone();
};

struct TOperatorQuery {
    std::vector<std::shared_ptr<IOa>> cols;

    TOperatorQuery Clone();
};

enum class EAoEngineType {
    kAgregation,
    kOperator
};

struct TAoQuery {
    std::vector<std::shared_ptr<IOa>> args;
    std::vector<std::pair<ui64, std::string>> aliases;
    EAoEngineType tp;

    TAoQuery Clone();
};

class IAoEngine {
public:
    virtual ~IAoEngine() = default;

    IAoEngine() = default;
    IAoEngine(std::vector<std::pair<ui64, std::string>> aliases);

    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual Expected<std::vector<TColumnPtr>> ThrowRowGroup() = 0;
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
    Expected<std::vector<TColumnPtr>> ThrowRowGroup() override;
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
    Expected<std::vector<TColumnPtr>> ThrowRowGroup() override;
    std::vector<std::string>& GetNames() override;
    std::shared_ptr<IAoEngine> Clone() override;

private:
    TAgregationQuery q_;
};

} // namespace JfEngine