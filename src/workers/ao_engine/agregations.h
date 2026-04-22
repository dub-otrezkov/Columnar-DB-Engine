#pragma once

#include "operators.h"

#include "columns/operators/operators.h"

#include "workers/io/io.h"

namespace JfEngine {

struct IAgregationOnly : public IOa {
    virtual ~IAgregationOnly() = default;

    IOa* arg;
    
    EAoType GetType() const override {
        return EAoType::kAgregation;
    }

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TSumAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

struct TMinAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

struct TMaxAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

struct TCountAgr : public IAgregationOnly {
    i64 ans = 0;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

struct TAvgAgr : public IAgregationOnly {
    TSumAgr sum;
    TCountAgr cnt;

    bool inited = false;

    std::string GetName() const override;
    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

} // namespace JfEngine
