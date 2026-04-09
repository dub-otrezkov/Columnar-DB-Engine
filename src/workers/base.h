#pragma once

#include "utils/errors/errors_templates.h"

#include "columns/types/types.h"

#include <vector>

namespace JfEngine {

struct TRowScheme {
    std::string name_;
    EColumn type_;
};

const ui64 kRowGroupLen = 10000;

class ITableInput {
public:
    ITableInput(ui64 row_group_len = kRowGroupLen);
    virtual ~ITableInput() = default;

    virtual std::vector<TRowScheme>& GetScheme();
    virtual Expected<std::vector<TColumnPtr>> ReadRowGroup();
    virtual Expected<IColumn> ReadColumn(const std::string& name);

    virtual Expected<void> SetupColumnsScheme() = 0;
    virtual Expected<std::vector<TColumnPtr>> LoadRowGroup() = 0;

    virtual ui64 GetGroupsCount() const;
    virtual ui64 GetRowGroupLen() const;
    virtual void MoveCursor();
    virtual void Reset();

protected:
    std::optional<Expected<std::vector<TColumnPtr>>> current_rg_;
    std::vector<TRowScheme> scheme_;
    ui64 row_group_len_;

    std::unordered_map<std::string, ui64> name_to_i_;
};

} // namespace JfEngine