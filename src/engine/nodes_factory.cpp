#include "nodes_factory.h"

#include <iostream>

namespace JFEngine {

TNodesFactory::TNodesFactory(std::vector<TRowScheme>& scheme) : scheme_(scheme) {}

Expected<IColumn> TNodesFactory::Make(
    ui64 ind,
    const std::string& value
) {
    auto type = scheme_[ind].type_;
    if (type == "string") {
        auto res = std::make_shared<TStringColumn>();
        // auto err = res->Set(value);
        // if (err.HasError()) {
        //     return err.GetError();
        // }
        return res;
    } else if (type == "int64") {
        auto res = std::make_shared<Ti64Column>();
        // auto err = res->Set(value);
        // if (err.HasError()) {
        //     return err.GetError();
        // }
        return res;
    }
    return MakeError<UnsupportedErr>();
}

} // namespace JFEngine
