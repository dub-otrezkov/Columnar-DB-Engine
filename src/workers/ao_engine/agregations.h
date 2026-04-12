#pragma once

#include "operators.h"

#include "columns/operators/operators.h"

#include "workers/io/io.h"

namespace JfEngine {

struct IAgregationOnly : public IOa {
    std::shared_ptr<IOa> arg;
    
    EAoType GetType() const override {
        return EAoType::kAgregation;
    }
};

struct TSumAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TMinAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TMaxAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TCountAgr : public IAgregationOnly {
    i64 ans = 0;

    std::string GetName() override;
    
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TAvgAgr : public IAgregationOnly {
    TSumAgr sum;
    TCountAgr cnt;

    bool inited = false;
    
    std::string GetName() override;
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

} // namespace JfEngine
