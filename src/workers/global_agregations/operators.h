#pragma once

#include "columns/operators/operators.h"

#include "workers/io/io.h"

namespace JFEngine {

enum EAgregationType {
    kSum,
    kColumn,
};

struct IAgregation {
    virtual Expected<IColumn> ReadRowGroup(ITableInput* inp) = 0;

    virtual void AddArg(std::shared_ptr<IAgregation>);
};

struct TColumnAgr : IAgregation {
    std::string name;

    TColumnAgr(std::string column_name);

    Expected<IColumn> ReadRowGroup(ITableInput* inp) override;
};

struct TSumAgr : IAgregation {
    std::shared_ptr<IAgregation> arg;

    Expected<IColumn> ReadRowGroup(ITableInput* inp) override;

    void AddArg(std::shared_ptr<IAgregation>) override;
};

} // namespace JFEngine
