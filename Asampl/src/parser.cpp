#include "pch.h"
#include "parser.h"
#include "tree.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>

using namespace Lexer;

static AstNodeType TokenType_toAstNodeType(TokenType type);

Tree *Parser::buid_tree() {

	Tree *tree = program();

	//std::cout << "HELLO" << std::endl;

	if (!get_error().empty()) {
		std::cout << get_error() << std::endl;
		return nullptr;

	}
	return tree;

}

#define TRACE_END() \
  parser->reduce_level();

#define TRACE_CALL() \
    this->increase_level(); \
	


static void traceLevel(int level) {
	for (int i = 0; i < level; i++) {
		putchar('.');
		putchar('.');
	}
}

bool Parser::eoi() {
	return this->get_iterator() == this->get_token_sequence()->end() ? true : false;
}


Tree *Parser::accept(TokenType type) {
	if (eoi()) return nullptr;
	//Token *token = Iterator_value(this->tokens);
	Token lexem = this->get_iterator_value();

	if (lexem.get_type() == type) {

		AstNodeType astType = TokenType_toAstNodeType(type);

		AstNode *node = new AstNode(astType, lexem.get_buffer());
		Tree *tree = new Tree(node);
		this->increase_iterator();
		return tree;
	}
	return nullptr;
}

Tree *Parser::expect(TokenType type) {
	Tree *tree = accept(type);

	if (tree != nullptr) {
		return tree;
	}
	std::string currentTokenType = eoi() ? "EOI" : to_string(get_iterator_value().get_type());
	int error_line = get_iterator_value().get_line();
	std::string message = "ERROR: expected " + to_string(type) + " got " + currentTokenType +". Line: " + std::to_string(error_line) + ".\n";

	//std::cout << message << std::endl;
	set_error(message);

	return nullptr;
}


bool Parser::ebnf_sequence(Tree *node_to_fill, grammar_rule_t rule) {
	/*while (rule(parser) && parser->error.empty())
		;
	return parser->error.empty() ? true : false;*/

	Tree *node = nullptr;

	while ((node = rule()) && get_error().empty()) {
		if (node == nullptr) return false;
		//nodes->ch.push_back(node);
		node_to_fill->children.push_back(node);
	}
	return get_error().empty() ? true : false;
}

Tree *Parser::ebnf_one_of(grammar_rule_t rules[], size_t length) {

	/*bool match = false;

	for (int i = 0; i < length && !match; i++) {
		GrammarRule rule = rules[i];
		match = rule(parser);
		if (!parser->error.empty()) return false;
	}
	return match;*/

	Tree *node = nullptr;
	for (int i = 0; i < length && !node; i++) {
		grammar_rule_t rule = rules[i];
		node = rule();
		if (!get_error().empty()) return nullptr;
	}
	return node;
}

Tree *Parser::ebnf_one_of_lexem(TokenType types[], size_t length) {
	/*bool match = true;

	for (int i = 0; i < typesLen && !match; i++) {
		match = accept(parser, types[i]);
	}
	return match;*/
	Tree *node = nullptr;
	for (int i = 0; i < length && !node; i++) {
		node = accept(types[i]);
	}
	return node;
}

Tree *Parser::ebnf_ap_main_rule(grammar_rule_t next, grammar_rule_t ap) {
	/*if (next(parser)) {
		ap(parser);
		return parser->error.empty() ? true : false;
	}
	return false;*/
	Tree *nextNode = next();
	if (nextNode) {
		Tree *apNode = ap();
		if (apNode) {
			apNode->children.insert(apNode->children.begin(), nextNode);
			return apNode;
		}
		return nextNode;
	}
	return nullptr;
}

Tree *Parser::ebnf_ap_recursive_rule(TokenType types[], size_t typesLen, grammar_rule_t next, grammar_rule_t ap) {
	/*bool accepted = ebnf_one_of_tokens(parser, types, typesLen);
	if (accepted) {
		return next(parser) && ap(parser);
	}
	return false;*/

	Tree *opNode = ebnf_one_of_lexem(types, typesLen);
	if (opNode == nullptr) return nullptr;

	Tree *node = nullptr;
	Tree *nextNode = next();
	Tree *apNode = ap();
	if (apNode) {
		apNode->children.insert(apNode->children.begin(), nextNode);
		node = apNode;
	}
	else {
		node = nextNode;
	}

	opNode->children.push_back(node);
	return opNode;
}

/*void parser_dec_level(Parser **parser) {
	//(*parser)->level--;
	(*parser)->reduce_level();
}*/


Tree *Parser::id() {
	TRACE_CALL();
	return accept(TokenType::NAME);
}

Tree *Parser::string() {
	TRACE_CALL();
	return accept(TokenType::STRING_LITERAL);
}

Tree *Parser::boolean() {
	TRACE_CALL();
	return accept(TokenType::LOGIC);
}

Tree *Parser::number() {
	TRACE_CALL();
	return accept(TokenType::NUMBER);
}

Tree *Parser::program() {
	/*TRACE_CALL();
	return accept(parser, TokenType::PROGRAM)

		&& expect(parser, TokenType::NAME)

		&& expect(parser, TokenType::LEFT_BRACE)
		&& libraries_section(parser)
		&& handlers_section(parser)
		&& renderers_section(parser)
		&& sources_section(parser)

		&& sets_section(parser)
		&& elements_section(parser)
		&& tuples_section(parser)
		&& aggregates_section(parser)

		&& actions_section(parser)

		&& expect(parser, TokenType::RIGHT_BRACE);*/
	TRACE_CALL();
	if (!accept(TokenType::PROGRAM)
		|| !expect(TokenType::NAME) || !expect(TokenType::LEFT_BRACE)) return nullptr;

	
	Tree *librariesNode = libraries_section();
	if (!librariesNode) {
		return nullptr;
	}
	

	Tree *handlersNode = handlers_section();
	if (!handlersNode) {
		return nullptr;
	}
	Tree *renderersNode = renderers_section();
	if (!renderersNode) {
		return nullptr;
	}
	Tree *sourcesNode = sources_section();
	if (!sourcesNode) {
		return nullptr;
	}

	Tree *setsNode = sets_section();
	if (!setsNode) {
		return nullptr;
	}

	Tree *elementsNode = elements_section();
	if (!elementsNode) {
		return nullptr;
	}

	Tree *tuplesNode = tuples_section();
	if (!tuplesNode) {
		return nullptr;
	}
	Tree *aggregatesNode = aggregates_section();
	if (!aggregatesNode) {
		return nullptr;
	}
	Tree *actionsNode = actions_section();
	if (!actionsNode) {
		return nullptr;
	}

	Tree *progNode = new Tree(new AstNode(AstNodeType_PROGRAM, "program"));

	progNode->children.push_back(librariesNode);
	progNode->children.push_back(handlersNode);
	progNode->children.push_back(renderersNode);
	progNode->children.push_back(sourcesNode);
	progNode->children.push_back(setsNode);

	progNode->children.push_back(elementsNode);
	progNode->children.push_back(tuplesNode);
	progNode->children.push_back(aggregatesNode);
	progNode->children.push_back(actionsNode);

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}

	return progNode;
}

Tree *Parser::libraries_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::LIBRARIES)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, library_import)
		&& expect(parser, TokenType::RIGHT_BRACE);*/

	
	if (!accept(TokenType::LIBRARIES)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *libraryNode = new Tree(new AstNode(AstNodeType_LIBRARIES, "libraries"));

	
	if (!ebnf_sequence(libraryNode, std::bind(&Parser::library_import, this))) {
		return nullptr;
	}

	
	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}

	return libraryNode;
}

Tree *Parser::library_import() {
	TRACE_CALL();
	/*return accept(parser, TokenType::NAME)
		&& expect(parser, TokenType::SEMICOLON);*/
	//std::cout << "library_import" << std::endl;
	Tree *nameNode = accept(TokenType::NAME);
	if (!nameNode) return nullptr;

	if (!expect(TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *libImport = new Tree(new AstNode(AstNodeType_LIB_IMPORT, "libImport"));
	libImport->children.push_back(nameNode);
	return libImport;
}

Tree *Parser::handlers_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::HANDLERS)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, TokenType::RIGHT_BRACE);*/

	if (!accept(TokenType::HANDLERS)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *handlersNode = new Tree(new AstNode(AstNodeType_HANDLERS, "handlers"));

	if (!ebnf_sequence(handlersNode, std::bind(&Parser::item_import, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return handlersNode;
}

Tree *Parser::renderers_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::RENDERERS)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, TokenType::RIGHT_BRACE);*/

	if (!accept(TokenType::RENDERERS)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *renderersNode = new Tree(new AstNode(AstNodeType_RENDERERS, "renderers"));

	if (!ebnf_sequence(renderersNode, std::bind(&Parser::item_import, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return renderersNode;
}

Tree *Parser::sources_section() {
	TRACE_CALL();

	/*return accept(parser, TokenType::SOURCES)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, TokenType::RIGHT_BRACE);*/
	if (!accept(TokenType::SOURCES)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *handlersNode = new Tree(new AstNode(AstNodeType_SOURCES, "handlers"));

	if (!ebnf_sequence(handlersNode, std::bind(&Parser::item_import, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return handlersNode;
}

Tree *Parser::sets_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::SETS)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, TokenType::RIGHT_BRACE);*/

	if (!accept(TokenType::SETS)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *setsNode = new Tree(new AstNode(AstNodeType_SETS, "sets"));

	if (!ebnf_sequence(setsNode, std::bind(&Parser::item_import, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return setsNode;
}

Tree *Parser::item_import() {
	TRACE_CALL();
	/*return accept(parser, TokenType::NAME)
		&& expect(parser, TokenType::FROM)
		&& expr(parser)
		&& expect(parser, TokenType::SEMICOLON);*/
	Tree *nameNode = accept(TokenType::NAME);
	if (!nameNode) return nullptr;

	if (!expect(TokenType::FROM)) {
		return nullptr;
	}

	Tree *dataNode = data();
	if (dataNode == nullptr) {
		return nullptr;
	}
	if (!expect(TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *itemImport = new Tree(new AstNode(AstNodeType_LIB_IMPORT, "itemImport"));
	itemImport->children.push_back(nameNode);
	itemImport->children.push_back(dataNode);
	return itemImport;
}

Tree *Parser::elements_section() {
	TRACE_CALL();
	
	/*return accept(parser, TokenType::ELEMENTS)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, element_declaration)
		&& expect(parser, TokenType::RIGHT_BRACE);*/
	if (!accept(TokenType::ELEMENTS)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *elementsNode = new Tree(new AstNode(AstNodeType_ELEMENTS, "elements"));

	if (!ebnf_sequence(elementsNode, std::bind(&Parser::element_declaration, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return elementsNode;
}


Tree *Parser::tuples_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::TUPLES)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, element_declaration)
		&& expect(parser, TokenType::RIGHT_BRACE);*/
	if (!accept(TokenType::TUPLES)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *tuplesNode = new Tree(new AstNode(AstNodeType_TUPLES, "tuples"));

	if (!ebnf_sequence(tuplesNode, std::bind(&Parser::element_declaration, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return tuplesNode;
}


Tree *Parser::aggregates_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::AGGREGATES)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, element_declaration)
		&& expect(parser, TokenType::RIGHT_BRACE);*/
	if (!accept(TokenType::AGGREGATES)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *aggregatesNode = new Tree(new AstNode(AstNodeType_AGGREGATES, "aggregates"));

	if (!ebnf_sequence(aggregatesNode, std::bind(&Parser::element_declaration, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return aggregatesNode;
}

Tree *Parser::element_declaration() {
	TRACE_CALL();
	/*return accept(parser, TokenType::NAME)
		&& expect(parser, TokenType::ASSIGN)
		&& data(parser)
		&& expect(parser, TokenType::SEMICOLON);*/
	Tree *nameNode = accept(TokenType::NAME);
	if (!nameNode) return nullptr;

	if (!expect(TokenType::ASSIGN)) {
		return nullptr;
	}

	Tree *dataNode = data();
	if (dataNode == nullptr) {
		return nullptr;
	}
	if (!expect(TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *elementImport = new Tree(new AstNode(AstNodeType_LIB_IMPORT, "elementImport"));
	elementImport->children.push_back(nameNode);
	elementImport->children.push_back(dataNode);
	return elementImport;
}


Tree *Parser::actions_section() {
	TRACE_CALL();
	/*return accept(parser, TokenType::ACTIONS)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, action)
		&& expect(parser, TokenType::RIGHT_BRACE);*/
	//if (!accept(parser, TokenType::ACTIONS)) return nullptr;
	if (!accept(TokenType::ACTIONS)
		|| !expect(TokenType::LEFT_BRACE)) return nullptr;

	Tree *actionsNode = new Tree(new AstNode(AstNodeType_ACTIONS, "actions"));

	if (!ebnf_sequence(actionsNode, std::bind(&Parser::action, this))) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}

	return actionsNode;

}

Tree *Parser::action() {
	TRACE_CALL();
	grammar_rule_t rules[] = {
			std::bind(&Parser::block_actions, this),
			std::bind(&Parser::expr_st, this),
			std::bind(&Parser::sequence_action, this),
			std::bind(&Parser::download_action, this),
			std::bind(&Parser::upload_action, this),
			std::bind(&Parser::render_action, this),
			std::bind(&Parser::if_action, this),
			std::bind(&Parser::while_action, this),
			std::bind(&Parser::switch_action, this),
			std::bind(&Parser::substitution_action, this),
			std::bind(&Parser::timeline_action, this),
			std::bind(&Parser::print_action, this)
	};
	//std::cout << (sizeof(rules) / sizeof(*rules)) << std::endl;
	return ebnf_one_of(rules, (sizeof(rules) / sizeof(*rules)));
}

Tree *Parser::block_actions() {
	/*TRACE_CALL();
	return expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, action)
		&& expect(parser, TokenType::RIGHT_BRACE);*/
	TRACE_CALL();
	if (!accept(TokenType::LEFT_BRACE)) return nullptr;

	Tree *blockNode = new Tree(new AstNode(AstNodeType_BLOCK, "block"));

	if (ebnf_sequence(blockNode, std::bind(&Parser::action, this))) {

		if (!expect(TokenType::RIGHT_BRACE)) {
			return nullptr;
		}
	}
	return blockNode;
}

Tree *Parser::sequence_action() {
	TRACE_CALL();
	/*return accept(parser, TokenType::SEQUENCE)
		&& expect(parser, TokenType::LEFT_BRACE)
		&& ebnf_sequence(parser, action)
		&& expect(parser, TokenType::RIGHT_BRACE)
		&& expect(parser, TokenType::SEMICOLON);*/

	if (!accept(TokenType::SEQUENCE)) return nullptr;

	Tree *blockNode = block_actions();
	if (blockNode == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::SEMICOLON)) return nullptr;


	Tree *sequenceNode = new Tree(new AstNode(AstNodeType_SEQUENCE, "sequence"));

	sequenceNode->children.push_back(blockNode);

	return sequenceNode;
}

Tree *Parser::download_action() {
	TRACE_CALL();
	/*return accept(parser, TokenType::DOWNLOAD)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::FROM)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::WITH)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::SEMICOLON);*/
	if (!accept(TokenType::DOWNLOAD)) return nullptr;

	Tree *idNode1 = expr();
	if (idNode1 == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::FROM)) return nullptr;

	Tree *idNode2 = expr();
	if (idNode2 == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::WITH)) return nullptr;

	Tree *idNode3 = expr();
	if (idNode3 == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::SEMICOLON)) return nullptr;

	Tree *downloadNode = new Tree(new AstNode(AstNodeType_DOWNLOAD, "download"));

	downloadNode->children.push_back(idNode1);
	downloadNode->children.push_back(idNode2);
	downloadNode->children.push_back(idNode3);

	return downloadNode;
}

Tree *Parser::upload_action() {
	TRACE_CALL();
	/*return accept(parser, TokenType::UPLOAD)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::TO)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::WITH)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::SEMICOLON);*/

	if (!accept(TokenType::UPLOAD)) return nullptr;

	Tree *idNode1 = id();
	if (idNode1 == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::TO)) return nullptr;

	Tree *idNode2 = id();
	if (idNode2 == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::WITH)) return nullptr;

	Tree *idNode3 = id();
	if (idNode3 == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::SEMICOLON)) return nullptr;

	Tree *uploadNode = new Tree(new AstNode(AstNodeType_UPLOAD, "upload"));

	uploadNode->children.push_back(idNode1);
	uploadNode->children.push_back(idNode2);
	uploadNode->children.push_back(idNode3);

	return uploadNode;
}

Tree *Parser::render_action() {
	TRACE_CALL();
	/*return accept(parser, TokenType::RENDER)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::WITH)
		&& expr(parser)
		&& expect(parser, TokenType::SEMICOLON);*/
	if (!accept(TokenType::RENDER)) return nullptr;

	Tree *idNode = id();
	if (idNode == nullptr) {
		return nullptr;
	}

	if (!accept(TokenType::WITH)) return nullptr;

	Tree *exprNode = expr();
	if (exprNode == nullptr) {
		return nullptr;
	}
	if (!accept(TokenType::SEMICOLON)) return nullptr;

	Tree *renderNode = new Tree(new AstNode(AstNodeType_RENDER, "render"));

	renderNode->children.push_back(idNode);
	renderNode->children.push_back(exprNode);

	return renderNode;
}

Tree *Parser::while_action() {
	TRACE_CALL();
	if (!accept(TokenType::WHILE)
		|| !expect(TokenType::LEFT_BRACKET)) return nullptr;

	Tree *exprNode = expr();
	if (!exprNode) {
		return nullptr;
	}
	if (!expect(TokenType::RIGHT_BRACKET)) {  
		return nullptr;
	}
	Tree *blockNode = block_actions();
	if (blockNode == nullptr) {
		return nullptr;
	}

	Tree *whileNode = new Tree(new AstNode(AstNodeType_WHILE, "while"));

	whileNode->children.push_back(exprNode);
	whileNode->children.push_back(blockNode);
	return whileNode;
}

Tree *Parser::switch_action() {
	if (!accept(TokenType::SWITCH)
		|| !expect(TokenType::LEFT_BRACKET)) return nullptr;

	Tree *exprNode = expr();

	if (!exprNode) {
		return nullptr;
	}
	if (!expect(TokenType::RIGHT_BRACKET)) {
		return nullptr;
	}
	if (!expect(TokenType::LEFT_BRACE)) {
		return nullptr;
	}

	Tree *switchNode = new Tree(new AstNode(AstNodeType_SWITCH, "switch"));
	switchNode->children.push_back(exprNode);

	if (!ebnf_sequence(switchNode, std::bind(&Parser::switch_operator, this))) {
		return nullptr;
	}

	if (accept(TokenType::DEFAULT)) {
		if (!expect(TokenType::COLON)) {
			return nullptr;
		}
		Tree *default_node = new Tree(new AstNode(AstNodeType_DEFAULT, "default"));
		Tree *blockNode = action();
		if (blockNode == nullptr || !get_error().empty()) {
			return nullptr;
		}
		default_node->children.push_back(blockNode);
		switchNode->children.push_back(default_node);
	}

	if (!expect(TokenType::RIGHT_BRACE)) {
		return nullptr;
	}
	return switchNode;
}

Tree *Parser::switch_operator() {
	if (!accept(TokenType::CASE)) return nullptr;

	Tree *exprNode = expr();

	if (!exprNode) {
		return nullptr;
	}

	if (!expect(TokenType::COLON)) {
		return nullptr;
	}

	Tree *actionNode = action();
	if (actionNode == nullptr) {
		return nullptr;
	}

	Tree *case_node = new Tree(new AstNode(AstNodeType_CASE, "case"));
	case_node->children.push_back(exprNode);
	case_node->children.push_back(actionNode);
	return case_node;
}

Tree *Parser::print_action() {
	if (!accept(TokenType::PRINT)
		|| !expect(TokenType::LEFT_BRACKET)) {
		return nullptr;
	}

	Tree *exprNode = expr();

	if (!exprNode) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACKET)
		|| !expect(TokenType::SEMICOLON)) {
		return nullptr;
	}

	Tree *printNode = new Tree(new AstNode(AstNodeType_PRINT, "print"));
	printNode->children.push_back(exprNode);
	return printNode;
}

Tree *Parser::if_action() {
	/*TRACE_CALL();
	return accept(parser, TokenType::IF)
		&& expect(parser, TokenType::LEFT_BRACKET)
		&& expr(parser)
		&& expect(parser, TokenType::RIGHT_BRACKET)
		&& block_actions(parser)
		&& (accept(parser, TokenType::ELSE) ? block_actions(parser) : true);*/
	TRACE_CALL();

	if (!accept(TokenType::IF)
		|| !expect(TokenType::LEFT_BRACKET)) return nullptr;
	Tree *exprNode = expr();

	if (!exprNode) {
		return nullptr;
	}

	if (!expect(TokenType::RIGHT_BRACKET)) {   
		return nullptr;
	}

	Tree *actionNode = action();

	if (actionNode == nullptr) {
		return nullptr;
	}
	Tree *ifNode = new Tree(new AstNode(AstNodeType_IF, "if"));

	ifNode->children.push_back(exprNode);
	ifNode->children.push_back(actionNode);

	if (accept(TokenType::ELSE)) {

		Tree *elseNode = action();
		if (elseNode == nullptr || !get_error().empty()) {
			return nullptr;
		}

		ifNode->children.push_back(elseNode);
	}
	return ifNode;
}

/*
static bool cases_action() {
	return accept(parser, TokenType::CASE)
		&& expr(parser)
		&& accept(parser, TokenType::OF)
		&&

}*/

Tree *Parser::substitution_action() {
	TRACE_CALL();
	/*return accept(parser, TokenType::SUBSTITUTE)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::FOR)
		&& expect(parser, TokenType::NAME)
		&& expect(parser, TokenType::WHEN)
		&& expr(parser)
		&& expect(parser, TokenType::SEMICOLON);*/

	if (!accept(TokenType::SUBSTITUTE)) return nullptr;

	Tree *idNode1 = expect(TokenType::NAME);
	if (!idNode1) return nullptr;

	if (!expect(TokenType::FOR)) return nullptr;

	Tree *idNode2 = expect(TokenType::NAME);
	if (!idNode2) return nullptr;

	if (!expect(TokenType::WHEN)) return nullptr;

	Tree *exprNode = expr();
	if (!exprNode) return nullptr;

	if (!expect(TokenType::SEMICOLON)) return nullptr;

	Tree *substitution_node = new Tree(new AstNode(AstNodeType_SUBSTITUTION, "substitution"));
	substitution_node->children.push_back(idNode1);
	substitution_node->children.push_back(idNode2);
	substitution_node->children.push_back(exprNode);
	return substitution_node;


}

Tree *Parser::timeline_action() {

	/*TRACE_CALL();
	return accept(parser, TokenType::TIMELINE)
		&& timeline_overload(parser)
		&& block_actions(parser);*/
	if (!accept(TokenType::TIMELINE)) return nullptr;

	Tree *exprNode = timeline_overload();
	if (exprNode == nullptr) {
		return nullptr;
	}

	Tree *actionNode = block_actions();
	if (actionNode == nullptr) {
		return nullptr;
	}

	Tree *timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE, "timeline"));
	timelineNode->children.push_back(exprNode);
	timelineNode->children.push_back(actionNode);
	return timelineNode;
}

Tree *Parser::timeline_overload() {
	TRACE_CALL();

	grammar_rule_t rules[] = {
		std::bind(&Parser::timeline_expr, this),
		std::bind(&Parser::timeline_as, this),
		std::bind(&Parser::timeline_until, this)
	};
	return ebnf_one_of(rules, 3);
}

Tree *Parser::timeline_expr() {
	TRACE_CALL();
	/*return accept(parser, TokenType::LEFT_BRACKET)
		&& expr(parser)
		&& expect(parser, TokenType::COLON)
		&& expr(parser)
		&& expect(parser, TokenType::COLON)
		&& expr(parser)
		&& expect(parser, TokenType::RIGHT_BRACKET);*/
	if (!accept(TokenType::LEFT_BRACKET)) return nullptr;

	Tree *exprNode1 = expr();
	if (exprNode1 == nullptr) {
		return nullptr;
	}
	if (!expect(TokenType::COLON)) return nullptr;

	Tree *exprNode2 = expr();
	if (exprNode2 == nullptr) {
		return nullptr;
	}
	if (!expect(TokenType::COLON)) return nullptr;

	Tree *exprNode3 = expr();
	if (exprNode3 == nullptr) {
		return nullptr;
	}
	if (!expect(TokenType::RIGHT_BRACKET)) return nullptr;

	Tree *timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE_EXPR, "timeline_expr"));
	timelineNode->children.push_back(exprNode1);
	timelineNode->children.push_back(exprNode2);
	timelineNode->children.push_back(exprNode3);
	return timelineNode;
}

Tree *Parser::timeline_as() {
	TRACE_CALL();
	/*return accept(parser, TokenType::AS)
		&& expect(parser, TokenType::NAME);*/
	if (!accept(TokenType::AS)) return nullptr;

	Tree *idNode = id();
	if (idNode == nullptr) {
		return nullptr;
	}
	Tree *timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE_AS, "timeline_as"));
	timelineNode->children.push_back(idNode);
	return timelineNode;
}

Tree *Parser::timeline_until() {
	TRACE_CALL();
	/*return accept(parser, TokenType::UNTIL)
		&& expr(parser);*/
	if (!accept(TokenType::UNTIL)) return nullptr;

	Tree *exprNode = expr();
	if (exprNode == nullptr) {
		return nullptr;
	}
	Tree *timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE_UNTIL, "timeline_until"));
	timelineNode->children.push_back(exprNode);
	return timelineNode;
}



Tree *Parser::expr() {
	TRACE_CALL();
	return assign();
}

Tree *Parser::expr_st() {
	/*TRACE_CALL();
	if (expr(parser)) {
		return expect(parser, TokenType::SEMICOLON);
	}
	return accept(parser, TokenType::SEMICOLON);*/
	TRACE_CALL();
	Tree *exprNode = expr();

	if (exprNode) {
		expect(TokenType::SEMICOLON);
	}else {
		accept(TokenType::SEMICOLON);
	}
	return exprNode;
}

Tree *Parser::assign() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::log_or, this), std::bind(&Parser::assign_ap, this));
}
Tree *Parser::assign_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::ASSIGN
	};
	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::log_or, this), std::bind(&Parser::assign_ap, this));
}

Tree *Parser::log_or() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::log_and, this), std::bind(&Parser::log_or_ap, this));
}

Tree *Parser::log_or_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::OR
	};
	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::log_and, this), std::bind(&Parser::log_or_ap, this));
}

Tree *Parser::log_and() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::eq, this), std::bind(&Parser::log_and_ap, this));
}

Tree *Parser::log_and_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::AND
	};
	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::eq, this), std::bind(&Parser::log_and_ap, this));
}

Tree *Parser::eq() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::rel, this), std::bind(&Parser::eq_ap, this));
}

Tree *Parser::eq_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::EQUAL,
		TokenType::NOTEQUAL
	};
	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::rel, this), std::bind(&Parser::eq_ap, this));
}

Tree *Parser::rel() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::add, this), std::bind(&Parser::rel_ap, this));
}

Tree *Parser::rel_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::MORE,
		TokenType::LESS,
		TokenType::LESS_OR_EQUAL,
		TokenType::MORE_OR_EQUAL

	};

	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::add, this), std::bind(&Parser::rel_ap, this));
}

Tree *Parser::add() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::mult, this), std::bind(&Parser::add_ap, this));
}
Tree *Parser::add_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::PLUS,
		TokenType::MINUS
	};
	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::mult, this), std::bind(&Parser::add_ap, this));
}

Tree *Parser::mult() {
	TRACE_CALL();
	return ebnf_ap_main_rule(std::bind(&Parser::unary, this), std::bind(&Parser::mult_ap, this));
}
Tree *Parser::mult_ap() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::MULT,
		TokenType::DIV,
		TokenType::MOD

	};
	return ebnf_ap_recursive_rule(arr, (sizeof(arr) / sizeof(*arr)), std::bind(&Parser::unary, this), std::bind(&Parser::mult_ap, this));
}

Tree *Parser::unary() {
	TRACE_CALL();
	TokenType arr[] = {
		TokenType::PLUS,
		TokenType::MINUS,
		TokenType::NOT
	};
	Tree *opNode = ebnf_one_of_lexem(arr, (sizeof(arr) / sizeof(*arr)));
	Tree *primNode = primary();

	if (opNode) {
		opNode->children.push_back(primNode);
		return opNode;
	}
	return primNode;
}

Tree *Parser::primary() {
	TRACE_CALL();
	grammar_rule_t arr[] = {
		std::bind(&Parser::number, this),
		std::bind(&Parser::string, this),
		std::bind(&Parser::var_or_call, this),
		std::bind(&Parser::parentheses, this)
	};
	return ebnf_one_of(arr, (sizeof(arr) / sizeof(*arr)));
}

Tree *Parser::var_or_call() {
	TRACE_CALL();
	Tree *varNode = id();

	Tree *argListNode = fn_call();

	if (argListNode) {
		varNode->children.push_back(argListNode);
	}
	return varNode;
}
Tree *Parser::parentheses() {
	TRACE_CALL();
	if (!accept(TokenType::LEFT_BRACKET)) return nullptr;
	Tree *exprNode = expr();
	expect(TokenType::RIGHT_BRACKET); // @todo mb 
	return exprNode;
}
Tree *Parser::fn_call() {
	if (!accept(TokenType::LEFT_BRACKET)) return nullptr;
	Tree *argListNode = arg_list();
	expect(TokenType::RIGHT_BRACKET); 
	return argListNode;
}

Tree *Parser::arg_list() {
	/*TRACE_CALL();
	return expr(parser)
		&& (accept(parser, TokenType::COMMA) ? arg_list(parser) : true);*/
	TRACE_CALL();
	Tree *exprNode = expr();
	Tree *argListNode = new Tree(new AstNode(AstNodeType_ARGLIST, "arglist"));

	if (exprNode != nullptr) {
		//List_add(argListNode->children, exprNode);
		argListNode->children.push_back(exprNode);
		while (true) {
			if (!accept(TokenType::COMMA)) break;

			exprNode = expr();
			if (exprNode) {
				argListNode->children.push_back(exprNode);
			}
			else {
				break;
			}
		}
	}
	return argListNode;
}

Tree *Parser::data() {
	TRACE_CALL();
	grammar_rule_t arr[] = {
		std::bind(&Parser::expr, this),
		std::bind(&Parser::arr_ini, this)
	};
	return ebnf_one_of(arr, 2);
}

Tree *Parser::arr_ini() {
	TRACE_CALL();
	if (!accept(TokenType::LEFT_BRACE)) return nullptr;
	Tree *argListNode = arg_list();
	expect(TokenType::RIGHT_BRACE); 
}

static AstNodeType TokenType_toAstNodeType(TokenType type) {
	switch (type)
	{
	case TokenType::ASSIGN: return AstNodeType_ASSIGN;
	case TokenType::PLUS: return AstNodeType_ADD;
	case TokenType::MINUS: return AstNodeType_SUB;
	case TokenType::MULT: return AstNodeType_MUL;
	case TokenType::DIV: return AstNodeType_DIV;
	case TokenType::MOD: return AstNodeType_MOD;
	case TokenType::EQUAL: return AstNodeType_EQUAL;
	case TokenType::NOTEQUAL: return AstNodeType_NOTEQUAL;
	case TokenType::NOT: return AstNodeType_NOT;
	case TokenType::MORE: return AstNodeType_MORE;
	case TokenType::LESS: return AstNodeType_LESS;
	case TokenType::MORE_OR_EQUAL: return AstNodeType_MORE_OR_EQUAL;
	case TokenType::LESS_OR_EQUAL: return AstNodeType_LESS_OR_EQUAL;
	case TokenType::AND: return AstNodeType_AND;
	case TokenType::OR: return AstNodeType_OR;

	case TokenType::NAME: return AstNodeType_ID;
	case TokenType::NUMBER: return AstNodeType_NUMBER;
	case TokenType::STRING_LITERAL: return AstNodeType_STRING;
	case TokenType::LOGIC: return AstNodeType_BOOL;

	default:
		return AstNodeType_UNKNOWN;
	}
}