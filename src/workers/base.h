#pragma once

#include "utils/errors/errors_templates.h"

#include "columns/types/types.h"

#include <vector>

namespace JFEngine {

struct TRowScheme {
    std::string name_;
    EColumn type_;
};

const ui64 KRowGroupLen = 100;

class ITableInput {
public:
    ITableInput(ui64 row_group_len = KRowGroupLen);
    virtual ~ITableInput() = default;

    virtual Expected<void> SetupColumnsScheme() = 0;
    virtual std::vector<TRowScheme>& GetScheme() = 0;
    virtual Expected<std::vector<TColumnPtr>> ReadRowGroup() = 0;
    virtual Expected<IColumn> ReadColumn(const std::string& name);
    virtual void MoveCursor(i64 delta);
    virtual void Reset();

    ui64 GetRowGroupLen() const;

protected:
    ui64 row_group_len_;
};

} // namespace JFEngine