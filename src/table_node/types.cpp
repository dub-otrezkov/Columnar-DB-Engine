#include "types.h"

namespace JFEngine {

Expected<void> Ti64Column::Setup(std::vector<std::string> data) {
    SetType(Ei64Column);
    for (const auto& s : data) {
        try {
            cols_.push_back(std::stoll(s));
        } catch (...) {
            return MakeError<NotAnIntErr>();
        }
    }
    return nullptr;
}

Expected<void> TStringColumn::Setup(std::vector<std::string> data) {
    SetType(EStringColumn);
    for (ui64 i = 0; i < data.size(); i++) {
        cols_.push_back(std::move(data[i]));
    }
    return nullptr;
}

Expected<IColumn> MakeColumn(std::vector<std::string> data, std::string type) {
    if (type == "int64") {
        auto res = std::make_shared<Ti64Column>();
        auto t = res->Setup(std::move(data));
        if (t.HasError()) {
            return t.GetError();
        }
        return res;
    } else if (type == "string") {
        auto res = std::make_shared<TStringColumn>();
        auto t = res->Setup(std::move(data));
        if (t.HasError()) {
            return t.GetError();
        }
        return res;
    }
}

Expected<IColumn> MakeColumnJF(std::vector<std::string> data, std::string type) {
    if (type == "int64") {
        auto res = std::make_shared<Ti64Column>();
        auto t = res->Setup(std::move(data));
        if (t.HasError()) {
            return t.GetError();
        }
        return res;
    } else if (type == "string") {
        auto res = std::make_shared<TStringColumn>();
        auto t = res->Setup(std::move(data));
        if (t.HasError()) {
            return t.GetError();
        }
        return res;
    }
}

} // namespace JFEngine
