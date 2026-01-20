#pragma once

#include "table_node/types.h"

#include "utils/errors/errors.h"
#include "table_node/types.h"

#include <memory>
#include <vector>

namespace JFEngine {

struct TRowScheme {
    std::string name_;
    std::string type_;
};

class UnsupportedErr : public IError {
public:
    std::string Print() const override {
        return "Unsupported move";
    }
};

class TNodesFactory {
public:
    TNodesFactory() = delete;
    TNodesFactory(std::vector<TRowScheme>& scheme);

    Expected<IColumn> Make(
        ui64 ind,
        const std::string& value
    );

private:
    std::vector<TRowScheme>& scheme_;
};

} // namespace JFEngine
