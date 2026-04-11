#pragma once

#include "operators.h"

#include "columns/operators/operators.h"

#include "workers/io/io.h"

namespace JfEngine {

struct TSumAgr : public IOa {
    std::shared_ptr<IColumn> ans;

    std::shared_ptr<IOa> arg;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TMinAgr : public IOa {
    std::shared_ptr<IColumn> ans;

    std::shared_ptr<IOa> arg;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TMaxAgr : public IOa {
    std::shared_ptr<IColumn> ans;

    std::shared_ptr<IOa> arg;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TCountAgr : public IOa {
    i64 ans = 0;

    std::shared_ptr<IOa> arg;

    std::string GetName() override;
    
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TAvgAgr : public IOa {
    TSumAgr sum;
    TCountAgr cnt;

    bool inited = false;

    std::shared_ptr<IOa> arg;
    
    std::string GetName() override;
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

} // namespace JfEngine
