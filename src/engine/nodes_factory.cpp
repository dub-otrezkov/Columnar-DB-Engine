#include "nodes_factory.h"

#include <iostream>

TNodesFactory::TNodesFactory(std::vector<TRowScheme>& scheme) : scheme_(scheme) {}

std::pair<std::shared_ptr<ITableNode>, IError*> TNodesFactory::Make(
    ui64 ind,
    const std::string& value
) {
    auto type = scheme_[ind].type_;
    if (type == "string") {
        auto res = std::make_shared<TStringNode>();
        auto err = res->Set(value);
        return {res, err};
    } else if (type == "int64") {
        auto res = std::make_shared<Ti64Node>();
        auto err = res->Set(value);
        return {res, err};
    }
    return {nullptr, new UnsupportedErr};
}
