#pragma once

#include "interpreter/value.h"

namespace Asampl::Interpreter {

struct VarScope : public std::enable_shared_from_this<VarScope> {
    std::unordered_map<std::string, ValuePtr> variables;
    std::shared_ptr<VarScope> parent_scope;

    VarScope(std::shared_ptr<VarScope> parent)
        : std::enable_shared_from_this<VarScope>()
        , variables{}
        , parent_scope{std::move(parent)}
    {
    }

    ValuePtr& get(const std::string& id);
    ValuePtr& create(const std::string& id);
    std::shared_ptr<VarScope> inherit();
    void free_variables();

    void print(std::ostream& output, size_t offset = 0) const;
};

}
