#include "pch.h"
#include "interpreter.h"
#include "matcher.h"


int Program::execute(const Tree* ast_tree) {
	auto ast_node = ast_tree->get_node();
	assert(ast_node->type_ == AstNodeType::PROGRAM);

	for (const auto& child : ast_tree->get_children()) {
		if (!error_.empty()) {
			//todo
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

void Program::execute_library_import(const Tree* ast_tree) {

}

void Program::execute_handler_import(const Tree* ast_tree) {

}

void Program::execute_renderer_declaration(const Tree* ast_tree) {

}

void Program::execute_source_declaration(const Tree* ast_tree) {

}

void Program::execute_set_declaration(const Tree* ast_tree) {

}

void Program::execute_element_declaration(const Tree* ast_tree) {

}

void Program::execute_tuple_declaration(const Tree* ast_tree) {

}

void Program::execute_aggregate_declaration(const Tree* ast_tree) {

}

void Program::execute_actions(const Tree* ast_tree) {

}
