#pragma once

#include "utils/faster_hashmap/hashset.h"

#include "columns/operators/operators.h"
#include "columns/operators/distinct.h"

#include "workers/io/io.h"

namespace JfEngine {

struct IOa {
    virtual std::shared_ptr<IOa> Clone() = 0;
    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual Expected<IColumn> ThrowRowGroup() = 0;

    virtual std::string GetName() = 0;

    virtual void AddArg(std::shared_ptr<IOa>) {}
};

struct TColumnOp : public IOa {
    std::string name;

    TColumnOp(std::string name_);

    std::shared_ptr<IColumn> ans;

    std::string GetName() override;
    
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;
};

struct TPlusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<std::shared_ptr<IOa>> args;

    std::string GetName() override;
    
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TMinusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<std::shared_ptr<IOa>> args;

    std::string GetName() override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TLengthOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::shared_ptr<IOa> arg;

    std::string GetName() override;
    
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

struct TDistinctOp : public IOa {
    TColumnPtr ans;

    TDistinctSets cur_sets;

    std::shared_ptr<IOa> arg;

    std::string GetName() override;
    
    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    void AddArg(std::shared_ptr<IOa>) override;
};

} // namespace JfEngine