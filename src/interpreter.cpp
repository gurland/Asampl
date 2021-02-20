#include "pch.h"
#include "interpreter.h"
#include "interpreter/timeline.h"
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

static bool is_value(AstNodeType t);

ValuePtr AbstractValue::from_literal(const AstNode* data_node) {
    switch (data_node->type_) {
    case AstNodeType::NUMBER:
        return std::make_shared<Value<double>>(stod(data_node->value_));
    case AstNodeType::BOOL:
        return std::make_shared<Value<bool>>(data_node->value_ == "true");
    case AstNodeType::STRING:
        return std::make_shared<Value<std::string>>(data_node->value_);
    default:
        return nullptr;
    }
}

int Program::execute(const Tree *ast_tree) {
#ifdef ASAMPL_ENABLE_PYTHON
    Py_Initialize();
    boost::python::numpy::initialize();
#endif

	auto ast_node = ast_tree->get_node();
	assert(ast_node->type_ == AstNodeType::PROGRAM);

	for (const auto& child : ast_tree->get_children()) {
		const auto child_node = child->get_node();
		switch (child_node->type_) {
		case AstNodeType::LIBRARIES: {
			execute_library_import(child);
			break;
		}
		case AstNodeType::HANDLERS: {
			execute_handler_import(child);
			break;
		}
		case AstNodeType::RENDERERS: {
			execute_renderer_declaration(child);
			break;
		}
		case AstNodeType::SOURCES: {
			execute_source_declaration(child);
			break;
		}
		case AstNodeType::SETS: {
			execute_set_declaration(child);
			break;
		}
		case AstNodeType::ELEMENTS: {
			execute_element_declaration(child);
			break;
		}
		case AstNodeType::TUPLES: {
			execute_tuple_declaration(child);
			break;
		}
		case AstNodeType::AGGREGATES: {
			execute_aggregate_declaration(child);
			break;
		}
		case AstNodeType::ACTIONS: {
#ifdef ASAMPL_ENABLE_PYTHON
           try {
               execute_actions(child);
           } catch (const boost::python::error_already_set&) {
               PyErr_Print();
               throw InterpreterException("Python error");
           }
#else
           execute_actions(child);
#endif
			break;
		}
		default: {
            throw InterpreterException("Unrecognized section");
		}
		}
	}

#ifdef ASAMPL_ENABLE_PYTHON
    Py_Finalize();
#endif

	return EXIT_SUCCESS;
}

void Program::execute_library_import(const Tree *ast_tree) {
	auto ast_node = ast_tree->get_node();
	assert(ast_node->type_ == AstNodeType::LIBRARIES);

	for (const auto& child : ast_tree->get_children()) {
		const bool matches = child->match(
            AstNodeType::LIB_IMPORT,
            AstNodeType::ID
		);
        assert(matches);

        const auto& lib_name = child->get_children()[0]->get_node()->value_;

        bool opened_library = false;
        for (const auto& dir : libraries_directories_) {
            try {
                auto library = open_library(dir / lib_name);

                for (auto [ name, func ] : library->get_functions()) {
                    add_function(lib_name + '_' + name, std::move(func));
                }

                libraries_.push_back(std::move(library));
                opened_library = true;
                break;
            } catch (const InterpreterException& e) {}
        }

        if (!opened_library) {
            throw InterpreterException("Library " + lib_name + " not found");
        }
    }
}

void Program::execute_handler_import(const Tree *ast_tree) {
    for (const auto& child : ast_tree->get_children()) {
		const bool matches = child->match(
            AstNodeType::ITEM_IMPORT,
            AstNodeType::ID,
            is_value
		);
        assert(matches);

        const auto& children = child->get_children();
        const auto id_node = children[0]->get_node();
        const auto data_node = children[1]->get_node();

        handlers_[id_node->value_] = open_handler(handlers_directory_ / data_node->value_);
    }
}

void Program::execute_renderer_declaration(const Tree *ast_tree) {

}

void Program::execute_source_declaration(const Tree *ast_tree) {
    for (const auto child : ast_tree->get_children()) {
        const auto& children = child->get_children();
        const auto id_node = children[0]->get_node();
        const auto data_node = children[1]->get_node();

        sources_[id_node->value_] = data_node->value_;
    }
}

void Program::execute_set_declaration(const Tree *ast_tree) {

}

void Program::execute_element_declaration(const Tree *ast_tree) {
	for (const auto& child : ast_tree->get_children()) {

		const bool matches = child->match(
			AstNodeType::ELEMENT_IMPORT,
			AstNodeType::ID,
			is_value
		);
        assert(matches);
		
        const auto& children = child->get_children();
        const auto id_node = children[0]->get_node();
        const auto data_node = children[1]->get_node();

        add_variable(id_node->value_, data_node);
	}
}

void Program::execute_tuple_declaration(const Tree *ast_tree) {

}

void Program::execute_aggregate_declaration(const Tree *ast_tree) {

}

void Program::execute_actions(const Tree *ast_tree) {
    for (const auto& child : ast_tree->get_children()) {
        evaluate_expression(child);
    }
}

#define BINARY_EXPR \
    auto left = evaluate_expression(children.at(0)); \
    auto right = evaluate_expression(children.at(1));

#define UNARY_EXPR \
    auto operand = evaluate_expression(children.at(0))

ValuePtr Program::evaluate_expression(const Tree* ast_tree) {
    const auto& children = ast_tree->get_children();


    switch (ast_tree->get_node()->type_) {
        case AstNodeType::ASSIGN: {
            const auto& variable_id = children.at(0)->get_node()->value_;
            auto variable = variables_.find(variable_id);
            if (variable == variables_.end()) {
                throw InterpreterException("Variable with id '" + variable_id + "' does not exist");
            }

            variable->second = evaluate_expression(children.at(1));
            return std::make_shared<UndefinedValue>();
        }

        case AstNodeType::ADD: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() + right->try_get<double>());
        }
        case AstNodeType::SUB: {
            if (children.size() == 1) {
                UNARY_EXPR;
                return std::make_shared<Value<double>>(-operand->try_get<double>());
            } else {
                BINARY_EXPR;
                return std::make_shared<Value<double>>(
                    left->try_get<double>() - right->try_get<double>());
            }
        }
        case AstNodeType::MUL: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() * right->try_get<double>());
        }
        case AstNodeType::DIV: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() / right->try_get<double>());
        }
        case AstNodeType::MOD: {
            assert(false && "unimplemented");
        }
        case AstNodeType::EQUAL: {
            BINARY_EXPR;
            switch (left->get_type()) {
            case ValueType::NUMBER:
                return std::make_shared<Value<bool>>(
                    left->try_get<double>() == right->try_get<double>());
            case ValueType::BOOL:
                return std::make_shared<Value<bool>>(
                    left->try_get<bool>() == right->try_get<bool>());
            case ValueType::STRING:
                return std::make_shared<Value<bool>>(
                    left->try_get<std::string>() == right->try_get<std::string>());
            default:
                assert(false && "unimplemented");
            }
        }
        case AstNodeType::NOTEQUAL: {
            BINARY_EXPR;
            switch (left->get_type()) {
            case ValueType::NUMBER:
                return std::make_shared<Value<bool>>(
                    left->try_get<double>() != right->try_get<double>());
            case ValueType::BOOL:
                return std::make_shared<Value<bool>>(
                    left->try_get<bool>() != right->try_get<bool>());
            case ValueType::STRING:
                return std::make_shared<Value<bool>>(
                    left->try_get<std::string>() != right->try_get<std::string>());
            default:
                assert(false && "unimplemented");
            }
        }
        case AstNodeType::NOT: {
            UNARY_EXPR;
            return std::make_shared<Value<bool>>(!operand->try_get<bool>());
        }
        case AstNodeType::MORE: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() > right->try_get<double>());
        }
        case AstNodeType::LESS: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() < right->try_get<double>());
        }
        case AstNodeType::MORE_OR_EQUAL: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() >= right->try_get<double>());
        }
        case AstNodeType::LESS_OR_EQUAL: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() <= right->try_get<double>());
        }
        case AstNodeType::AND: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<bool>() && right->try_get<bool>());
        }
        case AstNodeType::OR: {
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<bool>() || right->try_get<bool>());
        }
        case AstNodeType::NUMBER:
        case AstNodeType::STRING:
        case AstNodeType::BOOL:
        return AbstractValue::from_literal(ast_tree->get_node());

        case AstNodeType::ID: {
            auto arglist = ast_tree->find_child(AstNodeType::ARGLIST);
            if (arglist == nullptr) {
                return get_abstract_variable_value_by_id(ast_tree->get_node()->value_);
            } else {
                const auto& function_name = ast_tree->get_node()->value_;
                const auto function_it = functions_.find(function_name);
                if (function_it == functions_.end()) {
                    throw InterpreterException("Function " + function_name + " does not exist");
                }

                std::vector<ValuePtr> arguments;
                arguments.reserve(arglist->get_children().size());
                for (const auto& child : arglist->get_children()) {
                    auto arg = evaluate_expression(child);
                    if (arg->is_undefined())
                        return std::make_shared<UndefinedValue>();
                    arguments.emplace_back(arg);
                }

                return function_it->second(arguments);
            }
        }

        case AstNodeType::IF: {
            assert(children.size() >= 2);

            const auto& condition = children[0];
            const auto& block = children[1];

            if (evaluate_expression(condition)->try_get<bool>()) {
                evaluate_expression(block);
            } else if (children.size() >= 3) {
                const auto& else_block = children[2];
                evaluate_expression(else_block);
            }
            return std::make_shared<UndefinedValue>();
        }

        case AstNodeType::WHILE: {
            assert(children.size() >= 2);

            const auto& condition = children[0];
            const auto& block = children[1];

            while (evaluate_expression(condition)->try_get<bool>()) {
                evaluate_expression(block);
            }
            return std::make_shared<UndefinedValue>();
        }

        case AstNodeType::BLOCK: {
             for (const auto& child : children) {
                 evaluate_expression(child);
             }
             return std::make_shared<UndefinedValue>();
        }

        case AstNodeType::DOWNLOAD: {
            const auto& target = children[0]->get_node()->value_;
            auto variable_it = variables_.find(target);
            if (variable_it == variables_.end()) {
                throw InterpreterException("Variable with id '" + target + "' does not exist");
            }

            auto *adwnld = add_download(children[1]->get_node()->value_, children[2]->get_node()->value_);
            // TODO refactor maybe? not sure how it works
            auto response = adwnld->download();
            if (response.is_valid()) {
                variable_it->second = std::move(response.value);
            }
            return std::make_shared<UndefinedValue>();
        }
        case AstNodeType::TIMELINE: {
            auto matches = ast_tree->match(
                AstNodeType::TIMELINE,
                [](AstNodeType t) {
                    return t == AstNodeType::TIMELINE_EXPR ||
                           t == AstNodeType::TIMELINE_AS ||
                           t == AstNodeType::TIMELINE_UNTIL;
                },
                AstNodeType::BLOCK
            );
            assert(matches);

            active_timeline_ = std::make_shared<Timeline>(this);
            evaluate_expression(children[0]);

            auto res = false;
            do {
                res = active_timeline_->prepare_iteration();
                evaluate_expression(children[1]);
            } while(res);
            return std::make_shared<UndefinedValue>();
        }
        case AstNodeType::TIMELINE_EXPR: {
            int offset = 0;
            if (children[0]->get_node()->type_ != AstNodeType::DOWNLOAD) {
                active_timeline_->start = evaluate_expression(children[0])->try_get<double>();
                active_timeline_->end = evaluate_expression(children[1])->try_get<double>();
                offset = 2;
            }
            for (auto it = children.begin() + offset; it != children.end(); ++it) {
                const auto& child = *it;
                const auto& gchildren = child->get_children();
                const auto& target = gchildren[0]->get_node()->value_;
                auto variable_it = variables_.find(target);
                if (variable_it == variables_.end()) {
                    throw InterpreterException("Variable with id '" + target + "' does not exist");
                }
                variable_it->second = std::make_shared<UndefinedValue>();

                auto *adwnld = add_download(gchildren[1]->get_node()->value_, gchildren[2]->get_node()->value_);
                active_timeline_->add_download(adwnld, target);
            }
            return std::make_shared<UndefinedValue>();
        }

        default:
            throw InterpreterException("Unimplemented operation");
    }
}


#undef BINARY_EXPR
#undef UNARY_EXPR

void Program::load_stdlib() {
    for (const auto& [ name, func ] : get_stdlib_functions()) {
        add_function(name, func);
    }
}

ActiveDownload *Program::add_download(const std::string& _source_node, const std::string& _handler_node) {
	const auto& source_node = _source_node;
	const auto& handler_node = _handler_node;

	const auto& source_file = sources_.at(source_node);
	auto active_it = active_downloads_.find(std::make_pair(source_file, handler_node));
	if (active_it == active_downloads_.end()) {
		IHandler* handler = handlers_.at(handler_node).get();
		active_it = active_downloads_.emplace(
			std::pair(source_node, handler_node),
			ActiveDownload{source_file, *handler}).first;
	}
	return &active_it->second;
}

static bool is_value(AstNodeType t) {
	return
		t == AstNodeType::NUMBER ||
		t == AstNodeType::STRING ||
		t == AstNodeType::BOOL;
}
