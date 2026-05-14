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

    inline void ClearArgs() override {
        arg = nullptr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TSumAgr : public IAgregationOnly {
    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;
};

struct TMinAgr : public IAgregationOnly {
    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;
};

struct TMaxAgr : public IAgregationOnly {
    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;
};

struct TCountAgr : public IAgregationOnly {
    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;
};

struct TAvgAgr : public IAgregationOnly {
    TSumAgr sum;
    std::vector<ui64> count;

    ui64 RegisterResult() override {
        count.push_back(0);
        used.push_back(false);
        return used.size() - 1;
    }

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
        sum.AddArg(arg);
    }

    inline void ClearArgs() override {
        arg = nullptr;
        sum.ClearArgs();
    }

    bool inited = false;

    std::string GetName() const override;
    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;
    TColumnPtr ThrowRowGroup() override;
};

struct TCountDistinctAgr : public IAgregationOnly {
    std::vector<TDistinctSets> cur_sets;

    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

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

} // namespace JfEngine
