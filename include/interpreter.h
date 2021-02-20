#pragma once

#include <filesystem>
#include <type_traits>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>

#include "tree.h"
#include "interpreter/handler.h"
#include "interpreter/timeline.h"
#include "interpreter/exception.h"
#include "interpreter/value.h"
#include "interpreter/function.h"
#include "interpreter/library.h"

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
    ActiveDownload *add_download(const std::string& _source_node, const std::string& _handler_node);

    void load_stdlib();

    void set_handlers_directory(std::filesystem::path path);
    void add_libraries_directory(std::filesystem::path path);

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
    std::filesystem::path handlers_directory_;
    std::vector<std::filesystem::path> libraries_directories_;

	std::unordered_map<std::string, ValuePtr> variables_;
    std::unordered_map<std::string, std::unique_ptr<Handler>> handlers_;
    std::unordered_map<std::string, Function> functions_;
    std::vector<std::unique_ptr<ILibrary>> libraries_;
    std::map<std::pair<std::string, std::string>, ActiveDownload> active_downloads_;
    std::unordered_map<std::string, std::string> sources_;
	std::unordered_map<std::string, std::type_index> types_;

	std::shared_ptr<Timeline> active_timeline_;
private:
	friend class Timeline;
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

inline void Program::set_handlers_directory(std::filesystem::path path) {
    handlers_directory_ = path;
}

inline void Program::add_libraries_directory(std::filesystem::path path) {
    libraries_directories_.push_back(path);
}
