#include "base.h"

namespace JFEngine {

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

ui64 ITableInput::GetRowGroupLen() const {
    return row_group_len_;
}

Expected<TColumnPtr> ITableInput::ReadColumn(const std::string& name) {
    return MakeError<UnimplementedErr>();
}

} // namespace JFEngine