#pragma once

#include "types.h"
#include <memory>

class UnsupportedErr : public IError {
public:
    std::string Print() const override {
        return "Unsupported move";
    }
};

class TNodesFactory {
public:
    std::pair<std::shared_ptr<ITableNode>, IError*> Make(
        const std::string& type,
        const std::string& value
    );
};

auto GetNodesFactory();
