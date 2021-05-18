#pragma once

#include <filesystem>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>

#include "tree.h"
#include "interpreter/value.h"
#include "interpreter/handler.h"
#include "interpreter/timeline.h"
#include "interpreter/exception.h"
#include "interpreter/function.h"
#include "interpreter/library.h"
#include "interpreter/var_scope.h"

namespace Asampl::Interpreter {

class Program {
public:
    Program();
    ~Program();

    Program(const Program&) = delete;
    Program(Program&&) = delete;

    Handler::ActiveDownload *add_download(const std::string& _source_node, const std::string& _handler_node);

    void add_handlers_directory(std::filesystem::path path);
    void add_libraries_directory(std::filesystem::path path);

	void execute(const as_tree& ast_tree);
private:
    ValuePtr evaluate(const as_tree& tree, const std::shared_ptr<VarScope>& scope);
    ValuePtr& evaluate_id(const as_tree& tree, const std::shared_ptr<VarScope>& scope);
    void assign(const as_tree& target, const as_tree& value, const std::shared_ptr<VarScope>& scope);
    Function create_function(const as_tree& tree, const std::shared_ptr<VarScope>& scope);

private:
    std::vector<std::filesystem::path> handlers_directory;
    std::vector<std::filesystem::path> libraries_directories;

    HandlerValue import_handler(const std::string& from);
    ValuePtr import_library(const std::string& from);

    std::shared_ptr<VarScope> global_scope;
private:
	friend class Timeline::Timeline;
};

inline void Program::add_handlers_directory(std::filesystem::path path) {
    handlers_directory.push_back(path);
}

inline void Program::add_libraries_directory(std::filesystem::path path) {
    libraries_directories.push_back(path);
}

}

