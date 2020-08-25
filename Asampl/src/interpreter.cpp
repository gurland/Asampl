#include "pch.h"
#include "interpreter.h"
#include "matcher.h"



static bool is_value(AstNodeType t);

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


static bool is_value(AstNodeType t) {
	return
		t == AstNodeType::NUMBER ||
		t == AstNodeType::STRING ||
		t == AstNodeType::BOOL;
}
