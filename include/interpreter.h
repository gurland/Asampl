#pragma once

#include <type_traits>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>

#include "tree.h"
#include "interpreter/handler.h"
#include "interpreter/exception.h"
#include "interpreter/value.h"
#include "interpreter/function.h"

class Program {
public:
    ValuePtr get_abstract_variable_value_by_id(const std::string& id) {
		if (variables_.find(id) == variables_.end()) {
            throw InterpreterException("Variable with id '" + id + "' does not exist");
		}
		return variables_.at(id);
    }

	template<typename T>
	Value<T> *get_variable_value_by_id(const std::string &id) {
		if (variables_.find(id) == variables_.end()) {
            throw InterpreterException("Variable with id '" + id + "' does not exist");
		}
		return dynamic_cast<Value<T>>(variables_.at(id));
	}

	void add_variable(const std::string& id, const AstNode *data_node);
    void add_function(const std::string& id, Function func);

    void load_stdlib();

	int execute(const Tree *ast_tree);
private:
	void execute_library_import(const Tree *ast_tree);
	void execute_handler_import(const Tree *ast_tree);
	void execute_renderer_declaration(const Tree *ast_tree);
	void execute_source_declaration(const Tree *ast_tree);
	void execute_set_declaration(const Tree *ast_tree);
	void execute_element_declaration(const Tree *ast_tree);
	void execute_tuple_declaration(const Tree *ast_tree);
	void execute_aggregate_declaration(const Tree *ast_tree);
	void execute_actions(const Tree *ast_tree);

    ValuePtr evaluate_expression(const Tree* ast_tree);
private:


	//bool is_value(AstNodeType t);
private:
	std::unordered_map<std::string, ValuePtr> variables_;
    std::unordered_map<std::string, std::unique_ptr<Handler>> handlers_;
    std::unordered_map<std::string, Function> functions_;
    std::map<std::pair<std::string, std::string>, ActiveDownload> active_downloads_;
    std::unordered_map<std::string, std::string> sources_;

	std::unordered_map<std::string, std::type_index> types_;
};

inline void Program::add_variable(const std::string& id, const AstNode *data_node) {
    auto value = AbstractValue::from_literal(data_node);
    if (value != nullptr) {
        variables_.emplace(id, std::move(value));
    } else {
        throw InterpreterException("Invalid type of data node");
    }
}

inline void Program::add_function(const std::string& id, Function func) {
    functions_.emplace(id, func);
}
