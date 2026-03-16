#pragma once

#include "columns/operators/operators.h"

#include "workers/io/io.h"

namespace JFEngine {

enum EAgregationType {
    kSum,
    kColumn,
};

struct IAgregation {
    virtual std::shared_ptr<IAgregation> Clone() = 0;
    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual Expected<IColumn> ThrowRowGroup() = 0;

    virtual std::string GetName() = 0;

    virtual bool IsBlocker() = 0;

    virtual void AddArg(std::shared_ptr<IAgregation>);
};

struct TColumnAgr : IAgregation {
    std::string name;

    std::shared_ptr<IColumn> ans;

    std::string GetName() override;

    std::shared_ptr<IAgregation> Clone() override;

    TColumnAgr(std::string column_name);

    bool IsBlocker() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

struct TSumAgr : IAgregation {
    std::shared_ptr<IColumn> ans;

    std::shared_ptr<IAgregation> arg;

    std::string GetName() override;

    std::shared_ptr<IAgregation> Clone() override;

    bool IsBlocker() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IAgregation>) override;
};

struct TCountAgr : IAgregation {
    i64 ans = 0;

    std::shared_ptr<IAgregation> arg;

    std::string GetName() override;
    
    std::shared_ptr<IAgregation> Clone() override;

    bool IsBlocker() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IAgregation>) override;
};

struct TAvgAgr : IAgregation {
    TSumAgr sum;
    TCountAgr cnt;

    bool inited = false;

    std::string GetName() override;

    std::shared_ptr<IAgregation> arg;
    
    std::shared_ptr<IAgregation> Clone() override;

    bool IsBlocker() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IAgregation>) override;
};

} // namespace JFEngine
