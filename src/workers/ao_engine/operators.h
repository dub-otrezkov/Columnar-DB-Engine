#pragma once

#include "utils/faster_hashmap/hashset.h"

#include "columns/operators/operators.h"
#include "columns/operators/distinct.h"

#include "workers/io/io.h"

namespace JfEngine {

enum class EAoType {
    kAgregation,
    kOperator
};

struct IOa {
    virtual ~IOa() = default;

    virtual std::unique_ptr<IOa> Clone() = 0;
    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual Expected<IColumn> ThrowRowGroup() = 0;

    virtual std::string GetName() const = 0;

    virtual void AddArg(std::unique_ptr<IOa>) {}

    virtual EAoType GetType() const {
        return EAoType::kOperator;
    }
};

struct TColumnOp : public IOa {
    std::string name;

    std::shared_ptr<IColumn> ans;

    TColumnOp(std::string name_);

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    virtual inline bool IsTColumnOp() const {
        return true;
    }
};

struct TPlusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<std::shared_ptr<IOa>> args;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::unique_ptr<IOa> arg) override {
        args.push_back(std::move(arg));
    }
};

struct TMinusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<std::shared_ptr<IOa>> args;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::unique_ptr<IOa> arg) override {
        args.push_back(std::move(arg));
    }
};

struct TLengthOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::shared_ptr<IOa> arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::unique_ptr<IOa> to_agr) override {
        arg = std::move(to_agr);
    }
};

struct TDistinctOp : public IOa {
    TColumnPtr ans;

    TDistinctSets cur_sets;

    std::shared_ptr<IOa> arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(std::unique_ptr<IOa> to_agr) override {
        arg = std::move(to_agr);
    }
};

} // namespace JfEngine