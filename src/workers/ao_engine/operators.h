#pragma once

#include "columns/operators/distinct.h"
#include "columns/operators/operators.h"
#include "utils/faster_hashmap/hashset.h"
#include "workers/io/io.h"
#include "workers/filters/filter.h"

namespace JfEngine {

enum class EAoType {
    kAgregation,
    kOperator
};

struct IOa {
    virtual ~IOa() = default;

    bool is_final = false;

    virtual std::unique_ptr<IOa> Clone() = 0;
    virtual Expected<void> ConsumeRowGroup(ITableInput* inp) = 0;
    virtual TColumnPtr ThrowRowGroup() = 0;

    virtual std::string GetName() const = 0;

    virtual void AddArg(IOa*) {}

    virtual EAoType GetType() const {
        return EAoType::kOperator;
    }

    virtual inline bool IsConst() const {
        return false;
    }

    virtual inline const std::string& GetColumn() const = 0;
};

struct TColumnOp : public IOa {
    std::string name;
    i64 col_idx = -1;

    std::shared_ptr<IColumn> ans;

    TColumnOp(std::string name_);

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline const std::string& GetColumn() const override {
        return name;
    }
};

struct TPlusOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<IOa*> args;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

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

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

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

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TExtractMinuteOp : public IOa {
    std::shared_ptr<IColumn> ans;

    IOa* arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TTruncMinuteOp : public IOa {
    std::shared_ptr<IColumn> ans;

    IOa* arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TConstIntOp : public IOa {
    std::shared_ptr<IColumn> ans;

    IOa* arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }

    inline bool IsConst() const override {
        return true;
    }
};

struct TConstStrOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }

    inline bool IsConst() const override {
        return true;
    }
};

struct TDistinctOp : public IOa {
    TColumnPtr ans;

    TDistinctSets cur_sets;

    IOa* arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TIfOp : public IOa {
    TColumnPtr ans;

    TDistinctSets cur_sets;

    TFilterQuery cond;
    std::vector<IOa*> arg;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg.push_back(to_agr);
    }

    inline const std::string& GetColumn() const override {
        return arg[0]->GetColumn();
    }
};


struct TRegexpReplaceOp : public IOa {
    std::shared_ptr<IColumn> ans;

    std::vector<IOa*> arg;

    std::string arg1_p;
    std::string arg2_p;

    std::string GetName() const override;

    std::unique_ptr<IOa> Clone() override;

    Expected<void> ConsumeRowGroup(ITableInput* inp) override;
    TColumnPtr ThrowRowGroup() override;

    inline void AddArg(IOa* to_agr) override {
        arg.push_back(to_agr);
        if (arg.size() == 2) {
            arg1_p = to_agr->GetName();
        }
        if (arg.size() == 3) {
            arg2_p = to_agr->GetName();
        }
    }

    inline const std::string& GetColumn() const override {
        return arg[0]->GetColumn();
    }
};

} // namespace JfEngine