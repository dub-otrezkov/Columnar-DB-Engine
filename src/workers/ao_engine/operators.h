#pragma once

#include "columns/operators/distinct.h"
#include "columns/operators/operators.h"
#include "columns/operators/vector_like.h"
#include "utils/faster_hashmap/hashset.h"
#include "workers/io/io.h"
#include "workers/filters/filter.h"

namespace JfEngine {

enum class EAoType {
    kAgregation,
    kOperator
};

struct IOa {
    TColumnPtr ans;

    virtual ~IOa() = default;

    bool is_final = false;

    virtual Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) = 0;
    virtual TColumnPtr ThrowRowGroup() {
        return ans;
    }

    virtual std::string GetName() const = 0;

    virtual void AddArg(IOa*) {}
    virtual void ClearArgs() {}

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
    bool is_group_key = false;

    TColumnOp(std::string name_);

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline const std::string& GetColumn() const override {
        return name;
    }
};

struct TPlusOp : public IOa {
    std::vector<IOa*> args;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* arg) override {
        return args.push_back(arg);
    }

    inline void ClearArgs() override {
        args.clear();
    }

    inline const std::string& GetColumn() const override {
        return args[0]->GetColumn();
    }
};

struct TMinusOp : public IOa {
    std::vector<IOa*> args;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* arg) override {
        args.push_back(arg);
    }

    inline void ClearArgs() override {
        args.clear();
    }

    inline const std::string& GetColumn() const override {
        return args[0]->GetColumn();
    }
};

struct TLengthOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline void ClearArgs() override {
        arg = nullptr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TExtractMinuteOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline void ClearArgs() override {
        arg = nullptr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TTruncMinuteOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline void ClearArgs() override {
        arg = nullptr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TConstIntOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline void ClearArgs() override {
        arg = nullptr;
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

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline void ClearArgs() override {
        arg = nullptr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }

    inline bool IsConst() const override {
        return true;
    }
};

struct TIfOp : public IOa {
    TFilterQuery cond;
    std::vector<IOa*> arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg.push_back(to_agr);
    }

    inline void ClearArgs() override {
        arg.clear();
    }

    inline const std::string& GetColumn() const override {
        return arg[0]->GetColumn();
    }
};


struct TRegexpReplaceOp : public IOa {
    std::vector<IOa*> arg;

    std::string arg1_p;
    std::string arg2_p;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, std::vector<ui64>* idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg.push_back(to_agr);
        if (arg.size() == 2) {
            arg1_p = to_agr->GetName();
        }
        if (arg.size() == 3) {
            arg2_p = to_agr->GetName();
        }
    }

    inline void ClearArgs() override {
        arg.clear();
        arg1_p = "";
        arg2_p = "";
    }

    inline const std::string& GetColumn() const override {
        return arg[0]->GetColumn();
    }
};

} // namespace JfEngine