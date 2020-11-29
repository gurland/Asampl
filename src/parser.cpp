#include "pch.h"
#include "parser.h"
#include "tree.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>

template<typename T, std::size_t size>
std::size_t arraysize(T(&)[size]) { return size; }

using namespace Lexer;

static Tree *accept(Parser *parser, TokenType token);
static Tree *expect(Parser *parser, TokenType token);

static AstNodeType tokenType_to_astType(TokenType type);

static Tree *program(Parser *parser);

static Tree *libraries_section(Parser *parser);
static Tree *library_import(Parser *parser);

static Tree *handlers_section(Parser *parser);
static Tree *renderers_section(Parser *parser);


static Tree *sources_section(Parser *parser);
//static bool source_declaration(Parser *parser);

static Tree *sets_section(Parser *parser);

static Tree *item_import(Parser *parser);


static Tree *elements_section(Parser *parser);
static Tree *element_declaration(Parser *parser);

static Tree *tuples_section(Parser *parser);
//static bool tuples_declaration(Parser *parser);

static Tree *aggregates_section(Parser *parser);
//static bool aggregates_declaration(Parser *parser);


static Tree *actions_section(Parser *parser);
static Tree *action(Parser *parser);

static Tree *block_actions(Parser *parser);

static Tree *sequence_action(Parser *parser);
static Tree *download_action(Parser *parser);
static Tree *upload_action(Parser *parser);
static Tree *render_action(Parser *parser);
static Tree *if_action(Parser *parser);
static Tree *while_action(Parser *parser);
static Tree *switch_action(Parser *parser);
static Tree *switch_operator(Parser *parser);

static Tree *print_action(Parser *parser);

static Tree *substitution_action(Parser *parser);

static Tree *timeline_action(Parser *parser);
static Tree *timeline_overload(Parser *parser);

static Tree *timeline_expr(Parser *parser);
static Tree *timeline_as(Parser *parser);
static Tree *timeline_until(Parser *parser);
//static bool timeline_while(Parser *parser);


static Tree *expr(Parser *parser);
static Tree *expr_st(Parser *parser);
//static bool block_st(Parser *parser);


static Tree *assign(Parser *parser);
static Tree *assign_ap(Parser *parser);
static Tree *log_or(Parser *parser);
static Tree *log_or_ap(Parser *parser);
static Tree *log_and(Parser *parser);
static Tree *log_and_ap(Parser *parser);
static Tree *eq(Parser *parser);
static Tree *eq_ap(Parser *parser);
static Tree *rel(Parser *parser);
static Tree *rel_ap(Parser *parser);
static Tree *add(Parser *parser);
static Tree *add_ap(Parser *parser);
static Tree *mult(Parser *parser);
static Tree *mult_ap(Parser *parser);
static Tree *unary(Parser *parser);
static Tree *primary(Parser *parser);
static Tree *var_or_call(Parser *parser);
static Tree *parentheses(Parser *parser);
static Tree *fn_call(Parser *parser);
static Tree *arg_list(Parser *parser);
static Tree *data(Parser *parser);
static Tree *arr_ini(Parser *parser);

using grammar_rule_t = std::function<Tree *(Parser*)>;

//typedef Tree *(*GrammarRule)(Parser *parser);


Tree *Parser::buid_tree() {
	Tree *tree = program(this);

	if (!get_error().empty()) {
		std::cout << get_error() << std::endl;
		return nullptr;

	}
	return tree;
}

static bool eoi(Parser *parser) {
	return parser->get_iterator() == parser->get_token_sequence()->end() ? true : false;
}


static Tree *accept(Parser *parser, TokenType type) {
	if (eoi(parser)) return nullptr;
	Token lexem = parser->get_iterator_value();

	if (lexem.get_type() == type) {

		AstNodeType astType = tokenType_to_astType(type);

		AstNode *node = new AstNode(astType, lexem.get_buffer());
		Tree *tree = new Tree(node);
		parser->increase_iterator();
		return tree;
	}
	return nullptr;
}

static Tree *expect(Parser *parser, TokenType type) {
	Tree *tree = accept(parser, type);

	if (tree != nullptr) {
		return tree;
	}
	std::string currentTokenType = eoi(parser) ? "EOI" : to_string(parser->get_iterator_value().get_type());
	int error_line = parser->get_iterator_value().get_line();
	std::string message = "ERROR: expected " + to_string(type) + " got " + currentTokenType + ". Line: " + std::to_string(error_line) + ".\n";

	parser->set_error(message);

	return nullptr;
}


static bool ebnf_sequence(Parser *parser, Tree *node_to_fill, grammar_rule_t rule) {
	Tree *node = nullptr;

	while (node = rule(parser), node && parser->get_error().empty()) {
		if (node == nullptr) return false;
		//nodes->ch.emplace_back(node);
		node_to_fill->add_child(node);
	}
	return parser->get_error().empty() ? true : false;
}

static Tree *ebnf_one_of(Parser *parser, grammar_rule_t rules[], size_t length) {
	Tree *node = nullptr;
	for (int i = 0; i < length && !node; i++) {
		grammar_rule_t rule = rules[i];
		node = rule(parser);
		if (!parser->get_error().empty()) return nullptr;
	}
	return node;
}

static Tree *ebnf_one_of_lexem(Parser *parser, TokenType types[], size_t length) {
	Tree *node = nullptr;
	for (int i = 0; i < length && !node; i++) {
		node = accept(parser, types[i]);
	}
	return node;
}

static Tree *ebnf_ap_main_rule(Parser *parser, grammar_rule_t next, grammar_rule_t ap) {
	Tree *nextNode = next(parser);
	if (nextNode) {
		Tree *apNode = ap(parser);
		if (apNode) {
			apNode->add_child(apNode->get_children().cbegin(), nextNode);
			return apNode;
		}
		return nextNode;
	}
	return nullptr;
}

static Tree *ebnf_ap_recursive_rule(Parser *parser, TokenType types[], size_t typesLen, grammar_rule_t next, grammar_rule_t ap) {
	Tree *opNode = ebnf_one_of_lexem(parser, types, typesLen);
	if (opNode == nullptr) return nullptr;

	Tree *node = nullptr;
	Tree *nextNode = next(parser);
	Tree *apNode = ap(parser);
	if (apNode) {
		apNode->add_child(apNode->get_children().cbegin(), nextNode);
		node = apNode;
	}
	else {
		node = nextNode;
	}

	opNode->add_child(node);
	return opNode;
}

void parser_dec_level(Parser* *parser) {
	(*parser)->reduce_level();
}


static Tree *ID(Parser *parser) {
	parser->increase_level();
	return accept(parser, TokenType::NAME);
}

static Tree *String(Parser *parser) {
	parser->increase_level();
	return accept(parser, TokenType::STRING_LITERAL);
}

static Tree *BOOL(Parser *parser) {
	parser->increase_level();
	return accept(parser, TokenType::LOGIC);
}

static Tree *NUMBER(Parser *parser) {
	parser->increase_level();
	return accept(parser, TokenType::NUMBER);
}

static Tree *program(Parser *parser) {
	parser->increase_level();
	if (!expect(parser, TokenType::PROGRAM) ||
		!expect(parser, TokenType::NAME) ||
		!expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *prog_node = new Tree(new AstNode(AstNodeType::PROGRAM, "program"));

	Tree *libraries_node = libraries_section(parser);
	if (libraries_node) prog_node->add_child(libraries_node);
	
	Tree *handlers_node = handlers_section(parser);
	if (handlers_node) prog_node->add_child(handlers_node);

	Tree *renderers_node = renderers_section(parser);
	if (renderers_node) prog_node->add_child(renderers_node);

	Tree *sources_node = sources_section(parser);
	if (sources_node) prog_node->add_child(sources_node);

	Tree *sets_node = sets_section(parser);
	if (sets_node) prog_node->add_child(sets_node);

	Tree *elements_node = elements_section(parser);
	if (elements_node) prog_node->add_child(elements_node);

	Tree *tuples_node = tuples_section(parser);
	if (tuples_node) prog_node->add_child(tuples_node);

	Tree *aggregates_node = aggregates_section(parser);
	if (aggregates_node) prog_node->add_child(aggregates_node);

	Tree *actions_node = actions_section(parser);
	if (!actions_node) {
		//Tree::free(prog_node);
		return nullptr;
	}

	prog_node->add_child(actions_node);

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		//Tree::free(prog_node);
		return nullptr;
	}

	return prog_node;
}

static Tree *libraries_section(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::LIBRARIES)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *libraryNode = new Tree(new AstNode(AstNodeType::LIBRARIES, "libraries"));

	if (!ebnf_sequence(parser, libraryNode, library_import)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}

	return libraryNode;
}

static Tree *library_import(Parser *parser) {
	parser->increase_level();
	Tree *nameNode = accept(parser, TokenType::NAME);
	if (!nameNode) return nullptr;

	if (!expect(parser, TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *libImport = new Tree(new AstNode(AstNodeType::LIB_IMPORT, "libImport"));
	libImport->add_child(nameNode);
	return libImport;
}

static Tree *handlers_section(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::HANDLERS)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *handlersNode = new Tree(new AstNode(AstNodeType::HANDLERS, "handlers"));

	if (!ebnf_sequence(parser, handlersNode, item_import)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return handlersNode;
}

static Tree *renderers_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::RENDERERS)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *renderersNode = new Tree(new AstNode(AstNodeType::RENDERERS, "renderers"));

	if (!ebnf_sequence(parser, renderersNode, item_import)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return renderersNode;
}

static Tree *sources_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::SOURCES)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *handlersNode = new Tree(new AstNode(AstNodeType::SOURCES, "sources"));

	if (!ebnf_sequence(parser, handlersNode, item_import)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return handlersNode;
}

static Tree *sets_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::SETS)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *setsNode = new Tree(new AstNode(AstNodeType::SETS, "sets"));

	if (!ebnf_sequence(parser, setsNode, item_import)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return setsNode;
}

static Tree *item_import(Parser *parser) {
	parser->increase_level();
	Tree *nameNode = accept(parser, TokenType::NAME);
	if (!nameNode) return nullptr;

	if (!expect(parser, TokenType::FROM)) {
		return nullptr;
	}

	Tree *dataNode = data(parser);
	if (dataNode == nullptr) {
		return nullptr;
	}
	if (!expect(parser, TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *itemImport = new Tree(new AstNode(AstNodeType::ITEM_IMPORT, "itemImport"));
	itemImport->add_child(nameNode);
	itemImport->add_child(dataNode);
	return itemImport;
}

static Tree *elements_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::ELEMENTS)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *elementsNode = new Tree(new AstNode(AstNodeType::ELEMENTS, "elements"));

	if (!ebnf_sequence(parser, elementsNode, element_declaration)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return elementsNode;
}


static Tree *tuples_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::TUPLES)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *tuplesNode = new Tree(new AstNode(AstNodeType::TUPLES, "tuples"));

	if (!ebnf_sequence(parser, tuplesNode, element_declaration)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return tuplesNode;
}


static Tree *aggregates_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::AGGREGATES)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *aggregatesNode = new Tree(new AstNode(AstNodeType::AGGREGATES, "aggregates"));

	if (!ebnf_sequence(parser, aggregatesNode, element_declaration)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return aggregatesNode;
}

static Tree *element_declaration(Parser *parser) {
	parser->increase_level();
	Tree *nameNode = accept(parser, TokenType::NAME);
	if (!nameNode) return nullptr;

	if (!expect(parser, TokenType::ASSIGN)) {
		return nullptr;
	}

	Tree *dataNode = data(parser);
	if (dataNode == nullptr) {
		return nullptr;
	}
	if (!expect(parser, TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *elementImport = new Tree(new AstNode(AstNodeType::ELEMENT_IMPORT, "elementImport"));
	elementImport->add_child(nameNode);
	elementImport->add_child(dataNode);
	return elementImport;
}


static Tree *actions_section(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::ACTIONS)
		|| !expect(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *actionsNode = new Tree(new AstNode(AstNodeType::ACTIONS, "actions"));

	if (!ebnf_sequence(parser, actionsNode, action)) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}

	return actionsNode;

}

static Tree *action(Parser *parser) {
	parser->increase_level();
	grammar_rule_t rules[] = {
			block_actions,
			expr_st,
			sequence_action,
			download_action,
			upload_action,
			render_action,
			if_action,
			while_action,
			switch_action,
			substitution_action,
			timeline_action,
			print_action
	};

	return ebnf_one_of(parser, rules, arraysize(rules));
}

static Tree *block_actions(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::LEFT_BRACE)) return nullptr;

	Tree *blockNode = new Tree(new AstNode(AstNodeType::BLOCK, "block"));

	if (ebnf_sequence(parser, blockNode, action)) {

		if (!expect(parser, TokenType::RIGHT_BRACE)) {
			return nullptr;
		}
	}
	return blockNode;
}

static Tree *sequence_action(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::SEQUENCE)) return nullptr;

	Tree *blockNode = block_actions(parser);
	if (blockNode == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::SEMICOLON)) return nullptr;


	Tree *sequenceNode = new Tree(new AstNode(AstNodeType::SEQUENCE, "sequence"));

	sequenceNode->add_child(blockNode);

	return sequenceNode;
}

static Tree *download_action(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::DOWNLOAD)) return nullptr;

	Tree *idNode1 = expr(parser);
	if (idNode1 == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::FROM)) return nullptr;

	Tree *idNode2 = expr(parser);
	if (idNode2 == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::WITH)) return nullptr;

	Tree *idNode3 = expr(parser);
	if (idNode3 == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::SEMICOLON)) return nullptr;

	Tree *downloadNode = new Tree(new AstNode(AstNodeType::DOWNLOAD, "download"));

	downloadNode->add_child(idNode1);
	downloadNode->add_child(idNode2);
	downloadNode->add_child(idNode3);

	return downloadNode;
}

static Tree *upload_action(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::UPLOAD)) return nullptr;

	Tree *idNode1 = ID(parser);
	if (idNode1 == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::TO)) return nullptr;

	Tree *idNode2 = ID(parser);
	if (idNode2 == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::WITH)) return nullptr;

	Tree *idNode3 = ID(parser);
	if (idNode3 == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::SEMICOLON)) return nullptr;

	Tree *uploadNode = new Tree(new AstNode(AstNodeType::UPLOAD, "upload"));

	uploadNode->add_child(idNode1);
	uploadNode->add_child(idNode2);
	uploadNode->add_child(idNode3);

	return uploadNode;
}

static Tree *render_action(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::RENDER)) return nullptr;

	Tree *idNode = ID(parser);
	if (idNode == nullptr) {
		return nullptr;
	}

	if (!accept(parser, TokenType::WITH)) return nullptr;

	Tree *exprNode = expr(parser);
	if (exprNode == nullptr) {
		return nullptr;
	}
	if (!accept(parser, TokenType::SEMICOLON)) return nullptr;

	Tree *renderNode = new Tree(new AstNode(AstNodeType::RENDER, "render"));

	renderNode->add_child(idNode);
	renderNode->add_child(exprNode);

	return renderNode;
}

static Tree *while_action(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::WHILE)
		|| !expect(parser, TokenType::LEFT_BRACKET)) return nullptr;

	Tree *exprNode = expr(parser);
	if (!exprNode) {
		return nullptr;
	}
	if (!expect(parser, TokenType::RIGHT_BRACKET)) {
		return nullptr;
	}
	Tree *blockNode = block_actions(parser);
	if (blockNode == nullptr) {
		return nullptr;
	}

	Tree *whileNode = new Tree(new AstNode(AstNodeType::WHILE, "while"));

	whileNode->add_child(exprNode);
	whileNode->add_child(blockNode);
	return whileNode;
}

static Tree *switch_action(Parser *parser) {
	if (!accept(parser, TokenType::SWITCH)
		|| !expect(parser, TokenType::LEFT_BRACKET)) return nullptr;

	Tree *exprNode = expr(parser);

	if (!exprNode) {
		return nullptr;
	}
	if (!expect(parser, TokenType::RIGHT_BRACKET)) {
		return nullptr;
	}
	if (!expect(parser, TokenType::LEFT_BRACE)) {
		return nullptr;
	}

	Tree *switchNode = new Tree(new AstNode(AstNodeType::SWITCH, "switch"));
	switchNode->add_child(exprNode);

	if (!ebnf_sequence(parser, switchNode, switch_operator)) {
		return nullptr;
	}

	if (accept(parser, TokenType::DEFAULT)) {
		if (!expect(parser, TokenType::COLON)) {
			return nullptr;
		}
		Tree *default_node = new Tree(new AstNode(AstNodeType::DEFAULT, "default"));
		Tree *blockNode = action(parser);
		if (blockNode == nullptr || !parser->get_error().empty()) {
			return nullptr;
		}
		default_node->add_child(blockNode);
		switchNode->add_child(default_node);
	}

	if (!expect(parser, TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return switchNode;
}

static Tree *switch_operator(Parser *parser) {
	if (!accept(parser, TokenType::CASE)) return nullptr;

	Tree *exprNode = expr(parser);

	if (!exprNode) {
		return nullptr;
	}

	if (!expect(parser, TokenType::COLON)) {
		return nullptr;
	}

	Tree *actionNode = action(parser);
	if (actionNode == nullptr) {
		return nullptr;
	}

	Tree *case_node = new Tree(new AstNode(AstNodeType::CASE, "case"));
	case_node->add_child(exprNode);
	case_node->add_child(actionNode);
	return case_node;
}

static Tree *print_action(Parser *parser) {
	if (!accept(parser, TokenType::PRINT)
		|| !expect(parser, TokenType::LEFT_BRACKET)) {
		return nullptr;
	}

	Tree *exprNode = expr(parser);

	if (!exprNode) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACKET)
		|| !expect(parser, TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *printNode = new Tree(new AstNode(AstNodeType::PRINT, "print"));
	printNode->add_child(exprNode);
	return printNode;
}

static Tree *if_action(Parser *parser) {

	parser->increase_level();

	if (!accept(parser, TokenType::IF)
		|| !expect(parser, TokenType::LEFT_BRACKET)) return nullptr;
	Tree *exprNode = expr(parser);

	if (!exprNode) {
		return nullptr;
	}

	if (!expect(parser, TokenType::RIGHT_BRACKET)) {
		return nullptr;
	}

	Tree *actionNode = action(parser);

	if (actionNode == nullptr) {
		return nullptr;
	}
	Tree *ifNode = new Tree(new AstNode(AstNodeType::IF, "if"));

	ifNode->add_child(exprNode);
	ifNode->add_child(actionNode);

	if (accept(parser, TokenType::ELSE)) {

		Tree *elseNode = action(parser);
		if (elseNode == nullptr || !parser->get_error().empty()) {
			return nullptr;
		}

		ifNode->add_child(elseNode);
	}
	return ifNode;
}

static Tree *substitution_action(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::SUBSTITUTE)) return nullptr;

	Tree *idNode1 = expect(parser, TokenType::NAME);
	if (!idNode1) return nullptr;

	if (!expect(parser, TokenType::FOR)) return nullptr;

	Tree *idNode2 = expect(parser, TokenType::NAME);
	if (!idNode2) return nullptr;

	if (!expect(parser, TokenType::WHEN)) return nullptr;

	Tree *exprNode = expr(parser);
	if (!exprNode) return nullptr;

	if (!expect(parser, TokenType::SEMICOLON)) return nullptr;

	Tree *substitution_node = new Tree(new AstNode(AstNodeType::SUBSTITUTION, "substitution"));
	substitution_node->add_child(idNode1);
	substitution_node->add_child(idNode2);
	substitution_node->add_child(exprNode);
	return substitution_node;


}

static Tree *timeline_action(Parser *parser) {
	if (!accept(parser, TokenType::TIMELINE)) return nullptr;

	Tree *exprNode = timeline_overload(parser);
	if (exprNode == nullptr) {
		return nullptr;
	}

	Tree *actionNode = block_actions(parser);
	if (actionNode == nullptr) {
		return nullptr;
	}

	Tree *timelineNode = new Tree(new AstNode(AstNodeType::TIMELINE, "timeline"));
	timelineNode->add_child(exprNode);
	timelineNode->add_child(actionNode);
	return timelineNode;
}

static Tree *timeline_overload(Parser *parser) {
	parser->increase_level();

	grammar_rule_t rules[] = {
			timeline_expr,
			timeline_as,
			timeline_until
	};
	return ebnf_one_of(parser, rules, arraysize(rules));
}

static Tree *timeline_expr(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::LEFT_BRACKET)) return nullptr;

	Tree *timeline_node = new Tree(new AstNode(AstNodeType::TIMELINE_EXPR, "timeline_expr"));
	ebnf_sequence(parser, timeline_node, download_action);
	if (!expect(parser, TokenType::RIGHT_BRACKET)) return nullptr;
	return timeline_node;
}

static Tree *timeline_as(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::AS)) return nullptr;

	Tree *idNode = ID(parser);
	if (idNode == nullptr) {
		return nullptr;
	}
	Tree *timelineNode = new Tree(new AstNode(AstNodeType::TIMELINE_AS, "timeline_as"));
	timelineNode->add_child(idNode);
	return timelineNode;
}

static Tree *timeline_until(Parser *parser) {
	parser->increase_level();

	if (!accept(parser, TokenType::UNTIL)) return nullptr;

	Tree *exprNode = expr(parser);
	if (exprNode == nullptr) {
		return nullptr;
	}
	Tree *timelineNode = new Tree(new AstNode(AstNodeType::TIMELINE_UNTIL, "timeline_until"));
	timelineNode->add_child(exprNode);
	return timelineNode;
}



static Tree *expr(Parser *parser) {
	parser->increase_level();
	return assign(parser);
}

static Tree *expr_st(Parser *parser) {

	parser->increase_level();
	Tree *exprNode = expr(parser);

	if (exprNode) {
		expect(parser, TokenType::SEMICOLON);
	}
	else {
		accept(parser, TokenType::SEMICOLON);
	}
	return exprNode;
}

static Tree *assign(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, log_or, assign_ap);
}
static Tree *assign_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::ASSIGN
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), log_or, assign_ap);
}

static Tree *log_or(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, log_and, log_or_ap);
}

static Tree *log_or_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::OR
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), log_and, log_or_ap);
}

static Tree *log_and(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, eq, log_and_ap);
}

static Tree *log_and_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::AND
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), eq, log_and_ap);
}

static Tree *eq(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, rel, eq_ap);
}

static Tree *eq_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::EQUAL,
		TokenType::NOTEQUAL
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), rel, eq_ap);
}

static Tree *rel(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, add, rel_ap);
}

static Tree *rel_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::MORE,
		TokenType::LESS,
		TokenType::LESS_OR_EQUAL,
		TokenType::MORE_OR_EQUAL

	};

	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), add, rel_ap);
}

static Tree *add(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, mult, add_ap);
}
static Tree *add_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::PLUS,
		TokenType::MINUS
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), mult, add_ap);
}

static Tree *mult(Parser *parser) {
	parser->increase_level();
	return ebnf_ap_main_rule(parser, unary, mult_ap);
}
static Tree *mult_ap(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::MULT,
		TokenType::DIV,
		TokenType::MOD

	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), unary, mult_ap);
}

static Tree *unary(Parser *parser) {
	parser->increase_level();
	TokenType arr[] = {
		TokenType::PLUS,
		TokenType::MINUS,
		TokenType::NOT
	};
	Tree *opNode = ebnf_one_of_lexem(parser, arr, (sizeof(arr) / sizeof(*arr)));
	Tree *primNode = primary(parser);

	if (opNode) {
		opNode->add_child(primNode);
		return opNode;
	}
	return primNode;
}

static Tree *primary(Parser *parser) {
	parser->increase_level();
	grammar_rule_t rules[] = {
		NUMBER,
		String,
		var_or_call,
		parentheses
	};
	return ebnf_one_of(parser, rules, arraysize(rules));
}

static Tree *var_or_call(Parser *parser) {
	parser->increase_level();
	Tree *varNode = ID(parser);

	Tree *argListNode = fn_call(parser);

	if (argListNode) {
		varNode->add_child(argListNode);
	}
	return varNode;
}
static Tree *parentheses(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::LEFT_BRACKET)) return nullptr;
	Tree *exprNode = expr(parser);
	expect(parser, TokenType::RIGHT_BRACKET); // @todo mb
	return exprNode;
}
static Tree *fn_call(Parser *parser) {
	if (!accept(parser, TokenType::LEFT_BRACKET)) return nullptr;
	Tree *argListNode = arg_list(parser);
	expect(parser, TokenType::RIGHT_BRACKET);
	return argListNode;
}

static Tree *arg_list(Parser *parser) {
	/*parser->increase_level();
	return expr(parser)
		&& (accept(parser, TokenType::COMMA) ? arg_list(parser) : true);*/
	parser->increase_level();
	Tree *exprNode = expr(parser);
	Tree *argListNode = new Tree(new AstNode(AstNodeType::ARGLIST, "arglist"));

	if (exprNode != nullptr) {
		//List_add(argListNode->children_, exprNode);
		argListNode->add_child(exprNode);
		while (true) {
			if (!accept(parser, TokenType::COMMA)) break;

			exprNode = expr(parser);
			if (exprNode) {
				argListNode->add_child(exprNode);
			}
			else {
				break;
			}
		}
	}
	return argListNode;
}

static Tree *data(Parser *parser) {
	parser->increase_level();
	grammar_rule_t rules[] = {
		expr,
		arr_ini,
	};
	return ebnf_one_of(parser, rules, arraysize(rules));
}

static Tree *arr_ini(Parser *parser) {
	parser->increase_level();
	if (!accept(parser, TokenType::LEFT_BRACE)) return nullptr;
	Tree *argListNode = arg_list(parser);
	expect(parser, TokenType::RIGHT_BRACE);
}

static AstNodeType tokenType_to_astType(TokenType type) {
	switch (type)
	{
	case TokenType::ASSIGN: return AstNodeType::ASSIGN;
	case TokenType::PLUS: return AstNodeType::ADD;
	case TokenType::MINUS: return AstNodeType::SUB;
	case TokenType::MULT: return AstNodeType::MUL;
	case TokenType::DIV: return AstNodeType::DIV;
	case TokenType::MOD: return AstNodeType::MOD;
	case TokenType::EQUAL: return AstNodeType::EQUAL;
	case TokenType::NOTEQUAL: return AstNodeType::NOTEQUAL;
	case TokenType::NOT: return AstNodeType::NOT;
	case TokenType::MORE: return AstNodeType::MORE;
	case TokenType::LESS: return AstNodeType::LESS;
	case TokenType::MORE_OR_EQUAL: return AstNodeType::MORE_OR_EQUAL;
	case TokenType::LESS_OR_EQUAL: return AstNodeType::LESS_OR_EQUAL;
	case TokenType::AND: return AstNodeType::AND;
	case TokenType::OR: return AstNodeType::OR;
	case TokenType::NAME: return AstNodeType::ID;
	case TokenType::NUMBER: return AstNodeType::NUMBER;
	case TokenType::STRING_LITERAL: return AstNodeType::STRING;
	case TokenType::LOGIC: return AstNodeType::BOOL;
	default: return AstNodeType::UNKNOWN;
	}
}
