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
    boost::dynamic_bitset<> used;

    virtual ~IOa() = default;

    bool is_final = false;

    virtual ui64 RegisterResult() {
        used.push_back(false);
        return used.size() - 1;
    }

    virtual Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx = 0) = 0;
    virtual TColumnPtr ThrowRowGroup() {
        return ans;
    }

    virtual std::string GetName() const = 0;

    virtual void AddArg(IOa*) {}

    virtual EAoType GetType() const {
        return EAoType::kOperator;
    }

    virtual inline bool IsConst() const {
        return false;
    }

    virtual inline const std::string& GetColumn() const = 0;

    template<typename Combine>
    void CombineAt(ui64 idx, TColumnPtr value) {
        if (!ans) {
            ans = value;
            return;
        }
        if (used.size() < idx + 1) {
            used.resize(idx + 1, false);
        }
        if (ans->GetSize() < idx + 1) {
            Do<OResize>(ans, idx + 1);
        }
        if (used[idx]) {
            Do<Combine>(ans, idx, value);
        } else {
            Do<OSetAtIdx>(ans, idx, value);
            used[idx] = true;
        }
    }
};

struct TColumnOp : public IOa {
    std::string name;

    TColumnOp(std::string name_);

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline const std::string& GetColumn() const override {
        return name;
    }
};

struct TPlusOp : public IOa {
    std::vector<IOa*> args;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* arg) override {
        return args.push_back(arg);
    }

    inline const std::string& GetColumn() const override {
        return args[0]->GetColumn();
    }
};

struct TMinusOp : public IOa {
    std::vector<IOa*> args;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* arg) override {
        args.push_back(arg);
    }

    inline const std::string& GetColumn() const override {
        return args[0]->GetColumn();
    }
};

struct TLengthOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TExtractMinuteOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TTruncMinuteOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TConstIntOp : public IOa {
    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

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

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

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
    TDistinctSets cur_sets;

    IOa* arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg = to_agr;
    }

    inline const std::string& GetColumn() const override {
        return arg->GetColumn();
    }
};

struct TIfOp : public IOa {
    TFilterQuery cond;
    std::vector<IOa*> arg;

    std::string GetName() const override;

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

    inline void AddArg(IOa* to_agr) override {
        arg.push_back(to_agr);
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

    Expected<void> ConsumeRowGroup(ITableInput* inp, ui64 idx) override;

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