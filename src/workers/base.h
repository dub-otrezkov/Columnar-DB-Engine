#pragma once

#include "columns/types/types.h"
#include "utils/errors/errors_templates.h"

#include <boost/unordered/unordered_flat_map.hpp>

#include <vector>

namespace JfEngine {

struct TRowScheme {
    std::string name_;
    EColumn type_;
};

const ui64 kRowGroupLen = 100000;

class ITableInput;
using TTableInputPtr = std::shared_ptr<ITableInput>;

class ITableInput {
public:
    ITableInput(ui64 row_group_len = kRowGroupLen);
    virtual ~ITableInput() = default;

    virtual std::vector<TRowScheme>& GetScheme();
    virtual Expected<std::vector<TColumnPtr>> ReadRowGroup();
    virtual Expected<TColumnPtr> ReadColumn(const std::string& name);
    virtual Expected<TColumnPtr> ReadIthColumn(i64 i);
    virtual i64 GetColumnInd(const std::string& name);

    virtual Expected<void> SetupColumnsScheme() = 0;
    virtual Expected<std::vector<TColumnPtr>> LoadRowGroup() = 0;

    virtual ui64 GetRowGroupLen() const;
    virtual void MoveCursor();
    virtual void Reset();
    virtual const char* GetTypeName() const { return "Unknown"; }

private:
    void EnsureLoaded();

protected:
    std::shared_ptr<std::vector<TColumnPtr>> current_rg_;
    EError current_rg_err_ = EError::NoError;
    std::vector<TRowScheme> scheme_;
    ui64 row_group_len_;

    boost::unordered_flat_map<std::string, ui64> name_to_i_;
};

} // namespace JfEngine