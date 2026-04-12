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

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::shared_ptr<IOa> to_agr) override { arg = to_agr; }
};

struct TMinAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::shared_ptr<IOa> to_agr) override { arg = to_agr; }
};

struct TMaxAgr : public IAgregationOnly {
    std::shared_ptr<IColumn> ans;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::shared_ptr<IOa> to_agr) override { arg = to_agr; }
};

struct TCountAgr : public IAgregationOnly {
    i64 ans = 0;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::shared_ptr<IOa> to_agr) override { arg = to_agr; }
};

struct TAvgAgr : public IAgregationOnly {
    TSumAgr sum;
    TCountAgr cnt;

    bool inited = false;

    std::string GetName() const override;
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::shared_ptr<IOa> to_agr) override { arg = to_agr; }
};

} // namespace JfEngine
