#include "types.h"

namespace JFEngine {

TStringColumn::TStringColumn(std::vector<std::string> data) {
    cols_ = std::move(data);
}

EColumn TStringColumn::GetType() {
    return kStringColumn;
}

Expected<void> TStringColumn::Setup(std::vector<std::string> data) {
    for (ui64 i = 0; i < data.size(); i++) {
        cols_.push_back(std::move(data[i]));
    }
    return nullptr;
}

} // namespace JFEngine
