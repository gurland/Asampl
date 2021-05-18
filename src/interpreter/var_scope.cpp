#include <ostream>

#include "interpreter/var_scope.h"

namespace Asampl::Interpreter {

ValuePtr& VarScope::get(const std::string& id) {
    const auto it = variables.find(id);
    if (it != variables.end()) {
        return it->second;
    }

    if (parent_scope != nullptr)
    {
        return parent_scope->get(id);
    }
    
    throw InterpreterException("Variable '" + id + "' does not exist");
}

ValuePtr& VarScope::create(const std::string& id) {
    if (variables.find(id) != variables.end()) {
        throw InterpreterException("Variable '" + id + "' already exists in this block");
    }

    variables.insert(std::make_pair(id, ValuePtr{Undefined{}}));
    return variables.at(id);
}

std::shared_ptr<VarScope> VarScope::inherit() {
    return std::make_shared<VarScope>(shared_from_this());
}

void VarScope::print(std::ostream& output, size_t offset) const {
    std::string offset_str;
    offset_str.resize(offset, ' ');

    for (const auto& [ name, value ] : variables) {
        output << offset_str << name << " : " << value->to_string() << '\n';
    }

    if (parent_scope != nullptr) {
        parent_scope->print(output, offset + 1);
    }
}

void VarScope::free_variables() {
    variables.clear();
}

}
