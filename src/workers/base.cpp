#include "base.h"

namespace JFEngine {

ITableInput::ITableInput(ui64 row_group_len) : row_group_len_(row_group_len) {
}

ui64 ITableInput::GetRowGroupLen() const {
    return row_group_len_;
}

Expected<IColumn> ITableInput::ReadColumn(const std::string& name) {
    return MakeError<UnimplementedErr>();
}

void ITableInput::MoveCursor(i64) {
}

void ITableInput::Reset() {
}

} // namespace JFEngine