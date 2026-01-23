#pragma once

#include "errors.h"

#include <table_node/types.h>

#include <vector>

namespace JFEngine {

struct TRowScheme {
    std::string name_;
    std::string type_;
};

const ui64 KRowGroupLen = 100;

class ITableInput {
public:
    ITableInput(ui64 row_group_len = KRowGroupLen);
    virtual ~ITableInput() = default;

    virtual Expected<void> SetupColumnsScheme() = 0;
    virtual std::vector<TRowScheme>& GetScheme() = 0;
    virtual Expected<std::vector<TColumnPtr>> ReadRowGroup() = 0;
    virtual Expected<TColumnPtr> ReadColumn(const std::string& name);
    virtual void RestartDataRead() = 0;

    ui64 GetRowGroupLen() const;

protected:
    ui64 row_group_len_;
};

} // namespace JFEngine