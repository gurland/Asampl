#include "pch.h"
#include "interpreter.h"
#include "matcher.h"

#include <iomanip>
#include <cassert>
#include <cmath>
#include <ctime>

#include <opencv2/opencv.hpp>

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
	auto ast_node = ast_tree->get_node();
	assert(ast_node->type_ == AstNodeType::PROGRAM);

	for (const auto& child : ast_tree->get_children()) {
		if (!error_.empty()) {
			//todo
			return EXIT_FAILURE;
		}
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
			execute_actions(child);
			break;
		}
		default: {
			error_ = "Unrecognized section";
			break;
		}
		}
	}

	return EXIT_SUCCESS;
}

void Program::execute_library_import(const Tree *ast_tree) {

}

void Program::execute_handler_import(const Tree *ast_tree) {
    for (auto child : ast_tree->get_children()) {
		const bool matches = child->match(
			AstNodeType::ELEMENT_IMPORT,
			AstNodeType::ID,
			is_value
		);
        assert(matches);

        const auto children = child->get_children();
        const auto id_node = children[0]->get_node();
        const auto data_node = children[1]->get_node();

        handlers_[id_node->value_] = std::make_unique<Handler>(data_node->value_);
    }
}

void Program::execute_renderer_declaration(const Tree *ast_tree) {

}

void Program::execute_source_declaration(const Tree *ast_tree) {

}

void Program::execute_set_declaration(const Tree *ast_tree) {

}

void Program::execute_element_declaration(const Tree *ast_tree) {
	for (auto child : ast_tree->get_children()) {

		const bool matches = child->match(
			AstNodeType::ELEMENT_IMPORT,
			AstNodeType::ID,
			is_value
		);
        assert(matches);
		
        const auto children = child->get_children();
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
            auto variable = variables_.find(children.at(0)->get_node()->value_);
            if (variable == variables_.end()) {
                error_ = "Variable with such id does not exist";
                assert(false && "unimplemented");
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
            BINARY_EXPR;
            return std::make_shared<Value<double>>(
                left->try_get<double>() - right->try_get<double>());
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

        case AstNodeType::ID:
            return get_abstract_variable_value_by_id(ast_tree->get_node()->value_);

        default:
            assert(false && "unimplemented");
    }
}

#undef BINARY_EXPR
#undef UNARY_EXPR


static bool is_value(AstNodeType t) {
	return
		t == AstNodeType::NUMBER ||
		t == AstNodeType::STRING ||
		t == AstNodeType::BOOL;
}
