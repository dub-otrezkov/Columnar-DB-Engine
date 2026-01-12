#include "nodes_factory.h"

std::pair<std::shared_ptr<ITableNode>, IError*> TNodesFactory::Make(
    const std::string& type,
    const std::string& value
) {
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

auto GetNodesFactory() {
    static auto factory = std::make_shared<TNodesFactory>();
    return factory;
}
