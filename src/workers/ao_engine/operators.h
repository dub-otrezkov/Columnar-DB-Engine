#pragma once

#include "columns/operators/distinct.h"
#include "columns/operators/operators.h"
#include "utils/faster_hashmap/hashset.h"
#include "workers/io/io.h"

namespace JfEngine {

enum class EAoType {
    kAgregation,
    kOperator
};

struct IOa {
    virtual ~IOa() = default;

    bool is_final = false;

    virtual std::shared_ptr<IOa> Clone() = 0;
    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual Expected<IColumn> ThrowRowGroup() = 0;

    virtual std::string GetName() const = 0;

    virtual void AddArg(IOa*) {}

    virtual EAoType GetType() const {
        return EAoType::kOperator;
    }

    virtual inline const std::string& GetColumn() const = 0;
};

struct TColumnOp : public IOa {
    std::string name;

    std::shared_ptr<IColumn> ans;

    TColumnOp(std::string name_);

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline const std::string& GetColumn() const override {
        return name;
    }
};

struct TPlusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<IOa*> args;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(IOa* arg) override {
        return args.push_back(arg);
    }

    inline const std::string& GetColumn() const override {
        return args[0]->GetColumn();
    }
};

struct TMinusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<IOa*> args;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(IOa* arg) override {
        args.push_back(arg);
    }

    inline const std::string& GetColumn() const override {
        return args[0]->GetColumn();
    }
};

struct TLengthOp : public IOa {
    std::shared_ptr<IColumn> ans;

    IOa* arg;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TDistinctOp : public IOa {
    TColumnPtr ans;

    TDistinctSets cur_sets;

    IOa* arg;

    std::string GetName() const override;

    std::shared_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    Expected<IColumn> ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

} // namespace JfEngine