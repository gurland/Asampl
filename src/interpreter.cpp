#include "pch.h"
#include "interpreter.h"
#include "interpreter/timeline.h"
#include "interpreter/ffi_conversion.h"
#include "matcher.h"

#include <iomanip>
#include <cassert>
#include <cmath>
#include <ctime>
#include <fstream>
#include <future>

#include <opencv2/opencv.hpp>

#include "interpreter/test_stdlib.h"

#ifdef ASAMPL_ENABLE_PYTHON
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#endif

namespace Asampl::Interpreter {

static ValuePtr& dot_access(std::string path, const std::shared_ptr<VarScope>& scope) {
    std::optional<ValuePtr*> value;

    while (true) {
        const auto dot = path.find('.');
        if (dot == std::string::npos) {
            break;
        }
        const auto part = path.substr(0, dot);
        path = path.substr(dot + 1);

        if (value.has_value()) {
            value = &(**value)->get<Map>().get(part);
        } else {
            value = &scope->get(part);
        }
    }

    if (value.has_value()) {
        value = &(**value)->get<Map>().get(path);
    } else {
        value = &scope->get(path);
    }

    return *value.value();
}

Program::Program() {
#ifdef ASAMPL_ENABLE_PYTHON
    Py_Initialize();
    boost::python::numpy::initialize();
    init_conversions();
#endif
}

Program::~Program() {
#ifdef ASAMPL_ENABLE_PYTHON
    Py_Finalize();
#endif
}

void Program::execute(const as_tree& ast_tree) {
    global_scope = std::make_shared<VarScope>(nullptr);

    for (auto&& function : get_stdlib_functions()) {
        global_scope->create(function.name) = std::move(function);
    }

    for (const auto child : ast_tree.get_children()) {
        evaluate(*child, global_scope);
    }

    global_scope->get("main")->get<Function>().func({});
}

#define BINARY_EXPR \
    auto left = evaluate(*children.at(0), scope); \
    auto right = evaluate(*children.at(1), scope);

#define UNARY_EXPR \
    auto operand = evaluate(*children.at(0), scope)

#define UNARY(TYPE, OP) OP operand->get<TYPE>().value

#define BINARY(TYPE, OP) left->get<TYPE>().value OP right->get<TYPE>().value

struct CFReturn { ValuePtr value; };
struct CFBreak {};
struct CFContinue {};

ValuePtr Program::evaluate(const as_tree& tree, const std::shared_ptr<VarScope>& scope) {
    const auto& children = tree.get_children();
    const auto& node = tree.get_node();


    switch (tree.get_node()->type) {
        case ast_node_type::BLOCK: {
            auto block_scope = scope->inherit();
            ValuePtr ret = Undefined{};
            for (const auto& child : children) {
                ret = evaluate(*child, block_scope);
            }
            block_scope->free_variables();
            return ret;
        }

        case ast_node_type::IMPORT: {
            std::optional<std::string> id;
            std::optional<std::string> from;

            for (const auto& child : tree.get_children()) {
                if (child->match(ast_node_type::ID) || child->match(ast_node_type::STRING)) {
                    (id.has_value() ? from : id) = std::get<std::string>(child->get_node()->value);
                } else if (child->match(ast_node_type::FILE_NAME)) {
                    from = std::get<std::string>(child->get_node()->value);
                }
            }

            assert(id.has_value() && from.has_value());

            ValuePtr import;
            if (tree.find_child(ast_node_type::HANDLER) != nullptr) {
                auto handler = import_handler(*from);
                handler.name = *id;
                import = std::move(handler);
            } else {
                import = import_library(*from);
            }

            scope->create(*id) = std::move(import);

            return Undefined{};
        }

        case ast_node_type::FN: {
            auto func = create_function(tree, global_scope);
            *scope->create(func.name) = std::move(func);
            return Undefined{};
        }

        case ast_node_type::LAMBDA: {
            return create_function(tree, scope);
        }

        case ast_node_type::IF: {
            assert(children.size() >= 2);

            const auto& condition = children[0];
            const auto& block = children[1];

            if (evaluate(*condition, scope)->get<Bool>().value) {
                return evaluate(*block, scope);
            } else if (children.size() >= 3) {
                const auto& else_block = children[2];
                return evaluate(*else_block, scope);
            }
        }

        case ast_node_type::WHILE: {
            assert(children.size() >= 2);

            const auto& condition = children[0];
            const auto& block = children[1];

            while (evaluate(*condition, scope)->get<Bool>().value) {
                try {
                    evaluate(*block, scope);
                } catch (const CFBreak&) {
                    break;
                } catch (const CFContinue&) {
                    continue;
                }
            }
            return Undefined{};
        }

        // TODO match

        case ast_node_type::LET: {
            const auto& id = std::get<std::string>(children.at(0)->get_node()->value);
            scope->create(id);
            assign(*children.at(0), *children.at(1), scope);
            return Undefined{};
        }

        case ast_node_type::ASSIGNMENT: {
            assign(*children.at(1), *children.at(0), scope);
            return Undefined{};
        }

        case ast_node_type::PLUS: {
            BINARY_EXPR;
            return Number{BINARY(Number, +)};
        }
        case ast_node_type::MINUS: {
            if (children.size() == 1) {
                UNARY_EXPR;
                return Number{UNARY(Number, -)};
            } else {
                BINARY_EXPR;
                return Number{BINARY(Number, -)};
            }
        }
        case ast_node_type::MULT: {
            BINARY_EXPR;
            return Number{BINARY(Number, *)};
        }
        case ast_node_type::DIV: {
            BINARY_EXPR;
            return Number{BINARY(Number, /)};
        }
        case ast_node_type::MDIV: {
            assert(false && "unimplemented");
        }

        case ast_node_type::EQUAL: {
            BINARY_EXPR;
            return Bool{*left == *right};
        }
        case ast_node_type::NOT_EQUAL: {
            BINARY_EXPR;
            return Bool{*left != *right};
        }
        case ast_node_type::NOT: {
            UNARY_EXPR;
            return Bool{UNARY(Bool, !)};
        }
        case ast_node_type::MORE: {
            BINARY_EXPR;
            return Bool{BINARY(Number, <)};
        }
        case ast_node_type::LESS: {
            BINARY_EXPR;
            return Bool{BINARY(Number, >)};
        }
        case ast_node_type::MORE_EQUAL: {
            BINARY_EXPR;
            return Bool{BINARY(Number, <=)};
        }
        case ast_node_type::LESS_EQUAL: {
            BINARY_EXPR;
            return Bool{BINARY(Number, >=)};
        }
        case ast_node_type::LOG_AND: {
            BINARY_EXPR;
            return Bool{BINARY(Bool, &&)};
        }
        case ast_node_type::LOG_OR: {
            BINARY_EXPR;
            return Bool{BINARY(Bool, ||)};
        }
        case ast_node_type::NUMBER:
            return Number{std::get<double>(node->value)};
        case ast_node_type::STRING:
            return String{std::get<std::string>(node->value)};
        case ast_node_type::LOGIC:
            return Bool{static_cast<bool>(std::get<long>(node->value))};
        case ast_node_type::ARRAY: {
            Tuple value;
            for (const auto& child : tree.get_children()) {
                value.values.push_back(evaluate(*child, scope));
            }
            return value;
        }
        case ast_node_type::OBJ_DECL: {
            Map value;
            for (const auto& pair : tree.get_children()) {
                const auto& k = pair->get_children().at(0);
                const auto& v = pair->get_children().at(1);

                value.set(evaluate(*k, scope), evaluate(*v, scope));
            }

            return value;
        }

        case ast_node_type::ID: {
            return evaluate_id(tree, scope);
        }

        case ast_node_type::DOWNLOAD: {
            //const auto& target = std::get<std::string>(children[0]->get_node()->value);
            //auto& variable = scope->get(target);

            //auto *adwnld = add_download(
                //std::get<std::string>(children[1]->get_node()->value),
                //std::get<std::string>(children[2]->get_node()->value));

            //// TODO refactor maybe? not sure how it works
            //auto response = adwnld->download();
            //if (response.is_valid()) {
                //variable = std::move(response.value);
            //}
            return Undefined{};
        }
        case ast_node_type::TIMELINE: {
            auto params = evaluate(*children.at(0), scope)->get<Map>();
            auto callback = evaluate(*children.at(1), scope)->get<Function>();

            Timeline::Timeline timeline{params, callback};
            timeline.run();
            return Undefined{};
        }
        //case ast_node_type::TIMELINE_EXPR: {
            //int offset = 0;
            //if (children[0]->get_node()->type != ast_node_type::DOWNLOAD) {
                //active_timeline_->start = evaluate_expression(children[0])->get<Number>();
                //active_timeline_->end = evaluate_expression(children[1])->get<Number>();
                //offset = 2;
            //}
            //for (auto it = children.begin() + offset; it != children.end(); ++it) {
                //const auto& child = *it;
                //const auto& gchildren = child->get_children();
                //const auto& target = gchildren[0]->get_node()->value_;
                //auto variable_it = variables_.find(target);
                //if (variable_it == variables_.end()) {
                    //throw InterpreterException("Variable with id '" + target + "' does not exist");
                //}
                //variable_it->second = Undefined{};

                //auto *adwnld = add_download(gchildren[1]->get_node()->value_, gchildren[2]->get_node()->value_);
                //active_timeline_->add_download(adwnld, target);
            //}
            //return Undefined{};
        //}

        default:
            throw InterpreterException("Unimplemented operation");
    }
}

void Program::assign(const as_tree& target, const as_tree& value, const std::shared_ptr<VarScope>& scope) {
    const auto& id = std::get<std::string>(target.get_node()->value);
    auto& variable = scope->get(id);
    variable = evaluate(value, scope);
}

ValuePtr& Program::evaluate_id(const as_tree& tree, const std::shared_ptr<VarScope>& scope) {
    auto id = std::get<std::string>(tree.get_node()->value);
    auto& value = dot_access(id, scope);

    auto arglist = tree.find_child(ast_node_type::FN_CALL);
    auto array_el = tree.find_child(ast_node_type::ARR_EL);
    if (arglist == nullptr && array_el == nullptr) {
        return value;
    } else if (arglist != nullptr) {
        auto& func = value->get<Function>();

        std::vector<ValuePtr> arguments;
        arguments.reserve(arglist->get_children().size());
        for (const auto& child : arglist->get_children()) {
            arguments.emplace_back(evaluate(*child, scope));
        }

        static ValuePtr last_funcall_result;
        last_funcall_result = func.func(Utils::Slice{arguments});
        return last_funcall_result;
    } else {
        auto index = evaluate(*array_el->get_children().at(0), scope);
        if (value->is<Tuple>()) {
            return value->get<Tuple>()[index->get<Number>().as_int()];
        } else if (value->is<Map>()) {
            return value->get<Map>().get(index);
        } else {
            throw InterpreterException(id + " cannot be indexed because of its type");
        }
    }
}

Function Program::create_function(const as_tree& tree, const std::shared_ptr<VarScope>& scope) {
    const auto param_list = tree.find_child(ast_node_type::PARAM_LIST);
    const auto body = tree.find_child(ast_node_type::BLOCK);
    const auto name_node = tree.find_child(ast_node_type::ID);

    std::string name;
    if (name_node != nullptr) {
        name = std::get<std::string>(name_node->get_node()->value);
    } else {
        name = "lambda function";
    }

    std::function<ValuePtr(Utils::Slice<ValuePtr>)> func = [this, name, param_list, body, scope](auto args) {
        const auto func_scope = scope->inherit();
        if (param_list != nullptr) {
            for (size_t i = 0; i < param_list->get_children().size(); i++) {
                const auto child = param_list->get_children()[i];
                if (i < args.size()) {
                    func_scope->create(std::get<std::string>(child->get_node()->value)) = args[i];
                } else {
                    func_scope->create(std::get<std::string>(child->get_node()->value));
                }
            }
        }

        return evaluate(*body, func_scope);
    };

    return Function{
        std::move(name),
        std::move(func)
    };
}

HandlerValue Program::import_handler(const std::string& from) {
    for (const auto& dir : handlers_directory) {
        try {
            auto handler = Handler::open_handler(dir / from);
            return HandlerValue {
                from,
                std::move(handler)
            };
        } catch (InterpreterException& e) {}
    }

    throw InterpreterException("Handler " + from + " not found");
}

ValuePtr Program::import_library(const std::string& from) {
    for (const auto& dir : libraries_directories) {
        try {
            auto library = Library::open_library(dir / from);
            return library->get_value();
        } catch (InterpreterException& e) {}
    }

    throw InterpreterException("Library " + from + " not found");
}

#undef BINARY_EXPR
#undef UNARY_EXPR

//Handler::ActiveDownload *Program::add_download(const std::string& _source_node, const std::string& _handler_node) {
	//const auto& source_node = _source_node;
	//const auto& handler_node = _handler_node;

	//const auto& source_file = sources_.at(source_node);
	//auto active_it = active_downloads_.find(std::make_pair(source_file, handler_node));
	//if (active_it == active_downloads_.end()) {
        //Handler::IHandler* handler = handlers_.at(handler_node).get();
		//active_it = active_downloads_.emplace(
			//std::pair(source_node, handler_node),
			//Handler::ActiveDownload{source_file, *handler}).first;
	//}
	//return &active_it->second;
//}

}
