#include "pch.h"
#include "parser.h"
#include "tree.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

class Parser {
public:
	std::vector<Lexem>* lexem_sequence;
	std::vector<Lexem>::iterator iterator;
	std::string error;
	int level;
	
	//friend void TRACECALL();

	Parser() {
	}

	Lexem GetIteratorValue() {
		return *iterator;
	}
};



static Tree * accept(Parser * parser, LexemType token);
static Tree * expect(Parser * parser, LexemType token);

static AstNodeType TokenType_toAstNodeType(LexemType type);

static Tree * program(Parser * parser);

static Tree * libraries_section(Parser * parser);
static Tree * library_import(Parser * parser);

static Tree * handlers_section(Parser * parser);
static Tree * renderers_section(Parser * parser);


static Tree * sources_section(Parser * parser);
//static bool source_declaration(Parser * parser);

static Tree * sets_section(Parser * parser);

static Tree * item_import(Parser * parser);


static Tree * elements_section(Parser * parser);
static Tree * element_declaration(Parser * parser);

static Tree * tuples_section(Parser * parser);
//static bool tuples_declaration(Parser * parser);

static Tree * aggregates_section(Parser * parser);
//static bool aggregates_declaration(Parser * parser);


static Tree * actions_section(Parser * parser);
static Tree * action(Parser * parser);

static Tree * block_actions(Parser * parser);

static Tree * sequence_action(Parser * parser);
static Tree * download_action(Parser * parser);
static Tree * upload_action(Parser * parser);
static Tree * render_action(Parser * parser);
static Tree * if_action(Parser * parser);
static Tree * while_action(Parser * parser);
static Tree * switch_action(Parser * parser);
static Tree * switch_operator(Parser * parser);

static Tree * print_action(Parser * parser);

static Tree * substitution_action(Parser * parser);

static Tree * timeline_action(Parser * parser);
static Tree * timeline_overload(Parser * parser);

static Tree * timeline_expr(Parser * parser);
static Tree * timeline_as(Parser * parser);
static Tree * timeline_until(Parser * parser);
//static bool timeline_while(Parser * parser);


static Tree * expr(Parser * parser);
static Tree * expr_st(Parser * parser);
//static bool block_st(Parser * parser);


static Tree * assign(Parser * parser);
static Tree * assign_ap(Parser * parser);
static Tree * log_or(Parser * parser);
static Tree * log_or_ap(Parser * parser);
static Tree * log_and(Parser * parser);
static Tree * log_and_ap(Parser * parser);
static Tree * eq(Parser * parser);
static Tree * eq_ap(Parser * parser);
static Tree * rel(Parser * parser);
static Tree * rel_ap(Parser * parser);
static Tree * add(Parser * parser);
static Tree * add_ap(Parser * parser);
static Tree * mult(Parser * parser);
static Tree * mult_ap(Parser * parser);
static Tree * unary(Parser * parser);
static Tree * primary(Parser * parser);
static Tree * var_or_call(Parser * parser);
static Tree * parentheses(Parser * parser);
static Tree * fn_call(Parser * parser);
static Tree * arg_list(Parser * parser);
static Tree * data(Parser * parser);
static Tree * arr_ini(Parser * parser);

typedef Tree *(*GrammarRule)(Parser * parser);


Tree* parser_buid_tree(std::vector<Lexem>* lexem_sequence) {
	Parser parser;
	//parser.error = NULL;
	parser.lexem_sequence = lexem_sequence;
	parser.iterator = lexem_sequence->begin();
	parser.level = -1;

	Tree * tree = program(&parser);

	//std::cout << "HELLO" << std::endl;

	if (!parser.error.empty()) {
		std::cout << parser.error << std::endl;
		//	delete(parser.error);
		return NULL;

	}
	return tree;
	
}


#define TRACE_EXPECT(TYPE) \
   traceExpect(parser, TYPE);

// traceExpect(parser, TYPE);

#define TRACE_END()\
  parser->level--;

#define TRACE_CALL() \
    parser->level++; \
	trace(parser, __func__); 
	


static void traceLevel(int level) {
	for (int i = 0; i < level; i++) {
		putchar('.');
		putchar('.');
	}
}

void Parcer_decLevel(Parser ** parserPtr) {
	(*parserPtr)->level--;
}


static void traceExpect(Parser * parser, LexemType expectedType) {
	//traceLevel(parser->level);
	//printf("<%s>\n", LexemType_toString(expectedType));
	//std::cout << LexemType_toString(expectedType) << std::endl;
}

static bool eoi(Parser * parser) {
	return parser->iterator == parser->lexem_sequence->end() ? true : false;
}


static Tree * accept(Parser * self, LexemType type) {
	if (eoi(self)) return nullptr;
	//Token * token = Iterator_value(self->tokens);
	Lexem lexem = self->GetIteratorValue();

	if (lexem.GetType() == type) {

		AstNodeType astType = TokenType_toAstNodeType(type);

		AstNode * node = new AstNode(astType, lexem.GetBuffer());
		Tree * tree = new Tree(node);
		self->iterator++;
		return tree;
	}
	return NULL;
}

static Tree * expect(Parser * parser, LexemType type) {
	Tree * tree = accept(parser, type);

	if (tree!=NULL) {
		TRACE_EXPECT(type);
		return tree;
	}
	std::string currentTokenType = eoi(parser) ? "EOI" : LexemType_toString(parser->GetIteratorValue().GetType());
	int error_line = parser->GetIteratorValue().GetLine();
	std::string message = "ERROR: expected " + LexemType_toString(type) + " got " + currentTokenType +". Line: " + std::to_string(error_line) + ".\n";

	//std::cout << message << std::endl;
	parser->error = message;

	return NULL;
}


static bool ebnf_sequence(Parser * parser, Tree * node_to_fill, GrammarRule rule) {
	/*while (rule(parser) && parser->error.empty())
		;
	return parser->error.empty() ? true : false;*/

	Tree * node = NULL;

	while ((node = rule(parser)) && parser->error.empty()) {
		if (node == NULL) return false;
		//nodes->ch.push_back(node);
		node_to_fill->children.push_back(node);
	}
	return parser->error.empty() ? true : false;
}

static Tree * ebnf_one_of(Parser * parser, GrammarRule rules[], size_t length) {

	/*bool match = false;

	for (int i = 0; i < length && !match; i++) {
		GrammarRule rule = rules[i];
		match = rule(parser);
		if (!parser->error.empty()) return false;
	}
	return match;*/

	Tree * node = NULL;
	for (int i = 0; i < length && !node; i++) {
		GrammarRule rule = rules[i];
		node = rule(parser);
		if (!parser->error.empty()) return NULL;
	}
	return node;
}

static Tree * ebnf_one_of_lexem(Parser * parser, LexemType types[], size_t length) {
	/*bool match = true;

	for (int i = 0; i < typesLen && !match; i++) {
		match = accept(parser, types[i]);
	}
	return match;*/
	Tree * node = NULL;
	for (int i = 0; i < length && !node; i++) {
		node = accept(parser, types[i]);
	}
	return node;
}

static Tree * ebnf_ap_main_rule(Parser * parser, GrammarRule next, GrammarRule ap) {
	/*if (next(parser)) {
		ap(parser);
		return parser->error.empty() ? true : false;
	}
	return false;*/
	Tree * nextNode = next(parser);
	if (nextNode) {
		Tree * apNode = ap(parser);
		if (apNode) {
			apNode->children.insert(apNode->children.begin(), nextNode);
			return apNode;
		}
		return nextNode;
	}
	return NULL;
}

static Tree * ebnf_ap_recursive_rule(Parser * parser, LexemType types[], size_t typesLen, GrammarRule next, GrammarRule ap) {
	/*bool accepted = ebnf_one_of_tokens(parser, types, typesLen);
	if (accepted) {
		return next(parser) && ap(parser);
	}
	return false;*/

	Tree * opNode = ebnf_one_of_lexem(parser, types, typesLen);
	if (opNode == NULL) return NULL;

	Tree * node = NULL;
	Tree * nextNode = next(parser);
	Tree * apNode = ap(parser);
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


static void trace(Parser * parser, const char * fn) {
	//traceLevel(parser->level);
	//std::cout << fn << std::endl;
	//std::cout << "hello" << std::endl;
	if (eoi(parser)) {
		//printf("%s: <EOI>\n", fn);
		return;
	}

	Lexem lex = parser->GetIteratorValue();
	LexemType type = lex.GetType();
	switch (type)
	{

	case LexemType_NAME: case LexemType_NUMBER: case LexemType_STRING_LITERAL: {
		//std::cout << fn << ": <" << LexemType_toString(type) << "," << lex.GetBuffer() << ">\n";
		break;
	}
	default:
		//std::cout << fn << ": " << LexemType_toString(type) << std::endl;
		break;
	}
}

void parser_dec_level(Parser ** parser) {
	(*parser)->level--;
}


static Tree * ID(Parser * parser) {
	TRACE_CALL();
	return accept(parser, LexemType_NAME);
}

static Tree * String(Parser * parser) {
	TRACE_CALL();
	return accept(parser, LexemType_STRING_LITERAL);
}

static Tree * BOOL(Parser * parser) {
	TRACE_CALL();
	return accept(parser, LexemType_LOGIC);
}

static Tree * NUMBER(Parser * parser) {
	TRACE_CALL();
	return accept(parser, LexemType_NUMBER);
}

static Tree * program(Parser * parser) {
	/*TRACE_CALL();
	return accept(parser, LexemType_PROGRAM)

		&& expect(parser, LexemType_NAME)

		&& expect(parser, LexemType_LEFT_BRACE)
		&& libraries_section(parser)
		&& handlers_section(parser)
		&& renderers_section(parser)
		&& sources_section(parser)

		&& sets_section(parser)
		&& elements_section(parser)
		&& tuples_section(parser)
		&& aggregates_section(parser)

		&& actions_section(parser)

		&& expect(parser, LexemType_RIGHT_BRACE);*/
	TRACE_CALL();
	if (!accept(parser, LexemType_PROGRAM)
		|| !expect(parser, LexemType_NAME) || !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	
	Tree * librariesNode = libraries_section(parser);
	if (!librariesNode) {
		return NULL;
	}
	

	Tree * handlersNode = handlers_section(parser);
	if (!handlersNode) {
		return NULL;
	}
	Tree * renderersNode = renderers_section(parser);
	if (!renderersNode) {
		return NULL;
	}
	Tree * sourcesNode = sources_section(parser);
	if (!sourcesNode) {
		return NULL;
	}

	Tree * setsNode = sets_section(parser);
	if (!setsNode) {
		return NULL;
	}

	Tree * elementsNode = elements_section(parser);
	if (!elementsNode) {
		return NULL;
	}

	Tree * tuplesNode = tuples_section(parser);
	if (!tuplesNode) {
		return NULL;
	}
	Tree * aggregatesNode = aggregates_section(parser);
	if (!aggregatesNode) {
		return NULL;
	}
	Tree * actionsNode = actions_section(parser);
	if (!actionsNode) {
		return NULL;
	}

	Tree * progNode = new Tree(new AstNode(AstNodeType_PROGRAM, "program"));

	progNode->children.push_back(librariesNode);
	progNode->children.push_back(handlersNode);
	progNode->children.push_back(renderersNode);
	progNode->children.push_back(sourcesNode);
	progNode->children.push_back(setsNode);

	progNode->children.push_back(elementsNode);
	progNode->children.push_back(tuplesNode);
	progNode->children.push_back(aggregatesNode);
	progNode->children.push_back(actionsNode);

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}

	return progNode;
}

static Tree * libraries_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_LIBRARIES)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, library_import)
		&& expect(parser, LexemType_RIGHT_BRACE);*/

	
	if (!accept(parser, LexemType_LIBRARIES)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * libraryNode = new Tree(new AstNode(AstNodeType_LIBRARIES, "libraries"));

	
	if (!ebnf_sequence(parser, libraryNode, library_import)) {
		return NULL;
	}

	
	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}

	return libraryNode;
}

static Tree * library_import(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_NAME)
		&& expect(parser, LexemType_SEMICOLON);*/
	//std::cout << "library_import" << std::endl;
	Tree * nameNode = accept(parser,LexemType_NAME);
	if (!nameNode) return NULL;

	if (!expect(parser, LexemType_SEMICOLON)) {
		return NULL;
	}

	Tree * libImport = new Tree(new AstNode(AstNodeType_LIB_IMPORT, "libImport"));
	libImport->children.push_back(nameNode);
	return libImport;
}

static Tree * handlers_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_HANDLERS)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, LexemType_RIGHT_BRACE);*/

	if (!accept(parser, LexemType_HANDLERS)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * handlersNode = new Tree(new AstNode(AstNodeType_HANDLERS, "handlers"));

	if (!ebnf_sequence(parser, handlersNode, item_import)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return handlersNode;
}

static Tree * renderers_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_RENDERERS)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, LexemType_RIGHT_BRACE);*/

	if (!accept(parser, LexemType_RENDERERS)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * renderersNode = new Tree(new AstNode(AstNodeType_RENDERERS, "renderers"));

	if (!ebnf_sequence(parser, renderersNode, item_import)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return renderersNode;
}

static Tree * sources_section(Parser * parser) {
	TRACE_CALL();

	/*return accept(parser, LexemType_SOURCES)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, LexemType_RIGHT_BRACE);*/
	if (!accept(parser, LexemType_SOURCES)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * handlersNode = new Tree(new AstNode(AstNodeType_SOURCES, "handlers"));

	if (!ebnf_sequence(parser, handlersNode, item_import)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return handlersNode;
}

static Tree * sets_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_SETS)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, item_import)
		&& expect(parser, LexemType_RIGHT_BRACE);*/

	if (!accept(parser, LexemType_SETS)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * setsNode = new Tree(new AstNode(AstNodeType_SETS, "sets"));

	if (!ebnf_sequence(parser, setsNode, item_import)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return setsNode;
}

static Tree * item_import(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_NAME)
		&& expect(parser, LexemType_FROM)
		&& expr(parser)
		&& expect(parser, LexemType_SEMICOLON);*/
	Tree * nameNode = accept(parser, LexemType_NAME);
	if (!nameNode) return NULL;

	if (!expect(parser, LexemType_FROM)) {
		return NULL;
	}

	Tree * dataNode = data(parser);
	if (dataNode == NULL) {
		return NULL;
	}
	if (!expect(parser, LexemType_SEMICOLON)) {
		return NULL;
	}

	Tree * itemImport = new Tree(new AstNode(AstNodeType_LIB_IMPORT, "itemImport"));
	itemImport->children.push_back(nameNode);
	itemImport->children.push_back(dataNode);
	return itemImport;
}

static Tree * elements_section(Parser * parser) {
	TRACE_CALL();
	
	/*return accept(parser, LexemType_ELEMENTS)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, element_declaration)
		&& expect(parser, LexemType_RIGHT_BRACE);*/
	if (!accept(parser, LexemType_ELEMENTS)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * elementsNode = new Tree(new AstNode(AstNodeType_ELEMENTS, "elements"));

	if (!ebnf_sequence(parser, elementsNode, element_declaration)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return elementsNode;
}


static Tree * tuples_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_TUPLES)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, element_declaration)
		&& expect(parser, LexemType_RIGHT_BRACE);*/
	if (!accept(parser, LexemType_TUPLES)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * tuplesNode = new Tree(new AstNode(AstNodeType_TUPLES, "tuples"));

	if (!ebnf_sequence(parser, tuplesNode, element_declaration)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return tuplesNode;
}


static Tree * aggregates_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_AGGREGATES)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, element_declaration)
		&& expect(parser, LexemType_RIGHT_BRACE);*/
	if (!accept(parser, LexemType_AGGREGATES)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * aggregatesNode = new Tree(new AstNode(AstNodeType_AGGREGATES, "aggregates"));

	if (!ebnf_sequence(parser, aggregatesNode, element_declaration)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return aggregatesNode;
}

static Tree * element_declaration(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_NAME)
		&& expect(parser, LexemType_ASSIGN)
		&& data(parser)
		&& expect(parser, LexemType_SEMICOLON);*/
	Tree * nameNode = accept(parser, LexemType_NAME);
	if (!nameNode) return NULL;

	if (!expect(parser, LexemType_ASSIGN)) {
		return NULL;
	}

	Tree * dataNode = data(parser);
	if (dataNode == NULL) {
		return NULL;
	}
	if (!expect(parser, LexemType_SEMICOLON)) {
		return NULL;
	}

	Tree * elementImport = new Tree(new AstNode(AstNodeType_LIB_IMPORT, "elementImport"));
	elementImport->children.push_back(nameNode);
	elementImport->children.push_back(dataNode);
	return elementImport;
}


static Tree * actions_section(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_ACTIONS)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, action)
		&& expect(parser, LexemType_RIGHT_BRACE);*/
	//if (!accept(parser, LexemType_ACTIONS)) return NULL;
	if (!accept(parser, LexemType_ACTIONS)
		|| !expect(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * actionsNode = new Tree(new AstNode(AstNodeType_ACTIONS, "actions"));

	if (!ebnf_sequence(parser, actionsNode, action)) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}

	return actionsNode;

}

static Tree * action(Parser * parser) {
	TRACE_CALL();
	GrammarRule rules[] = {
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
	//std::cout << (sizeof(rules) / sizeof(*rules)) << std::endl;
	return ebnf_one_of(parser, rules, (sizeof(rules) / sizeof(*rules)));
}

static Tree * block_actions(Parser * parser) {
	/*TRACE_CALL();
	return expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, action)
		&& expect(parser, LexemType_RIGHT_BRACE);*/
	TRACE_CALL();
	if (!accept(parser, LexemType_LEFT_BRACE)) return NULL;

	Tree * blockNode = new Tree(new AstNode(AstNodeType_BLOCK, "block"));

	if (ebnf_sequence(parser, blockNode, action)) {

		if (!expect(parser, LexemType_RIGHT_BRACE)) {
			return NULL;
		}
	}
	return blockNode;
}

static Tree * sequence_action(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_SEQUENCE)
		&& expect(parser, LexemType_LEFT_BRACE)
		&& ebnf_sequence(parser, action)
		&& expect(parser, LexemType_RIGHT_BRACE)
		&& expect(parser, LexemType_SEMICOLON);*/

	if (!accept(parser, LexemType_SEQUENCE)) return NULL;

	Tree * blockNode = block_actions(parser);
	if (blockNode == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_SEMICOLON)) return NULL;


	Tree * sequenceNode = new Tree(new AstNode(AstNodeType_SEQUENCE, "sequence"));

	sequenceNode->children.push_back(blockNode);

	return sequenceNode;
}

static Tree * download_action(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_DOWNLOAD)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_FROM)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_WITH)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_SEMICOLON);*/
	if (!accept(parser, LexemType_DOWNLOAD)) return NULL;

	Tree * idNode1 = expr(parser);
	if (idNode1 == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_FROM)) return NULL;

	Tree * idNode2 = expr(parser);
	if (idNode2 == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_WITH)) return NULL;

	Tree * idNode3 = expr(parser);
	if (idNode3 == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_SEMICOLON)) return NULL;

	Tree * downloadNode = new Tree(new AstNode(AstNodeType_DOWNLOAD, "download"));

	downloadNode->children.push_back(idNode1);
	downloadNode->children.push_back(idNode2);
	downloadNode->children.push_back(idNode3);

	return downloadNode;
}

static Tree * upload_action(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_UPLOAD)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_TO)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_WITH)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_SEMICOLON);*/

	if (!accept(parser, LexemType_UPLOAD)) return NULL;

	Tree * idNode1 = ID(parser);
	if (idNode1 == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_TO)) return NULL;

	Tree * idNode2 = ID(parser);
	if (idNode2 == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_WITH)) return NULL;

	Tree * idNode3 = ID(parser);
	if (idNode3 == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_SEMICOLON)) return NULL;

	Tree * uploadNode = new Tree(new AstNode(AstNodeType_UPLOAD, "upload"));

	uploadNode->children.push_back(idNode1);
	uploadNode->children.push_back(idNode2);
	uploadNode->children.push_back(idNode3);

	return uploadNode;
}

static Tree * render_action(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_RENDER)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_WITH)
		&& expr(parser)
		&& expect(parser, LexemType_SEMICOLON);*/
	if (!accept(parser, LexemType_RENDER)) return NULL;

	Tree * idNode = ID(parser);
	if (idNode == NULL) {
		return NULL;
	}

	if (!accept(parser, LexemType_WITH)) return NULL;

	Tree * exprNode = expr(parser);
	if (exprNode == NULL) {
		return NULL;
	}
	if (!accept(parser, LexemType_SEMICOLON)) return NULL;

	Tree * renderNode = new Tree(new AstNode(AstNodeType_RENDER, "render"));

	renderNode->children.push_back(idNode);
	renderNode->children.push_back(exprNode);

	return renderNode;
}

static Tree * while_action(Parser * parser) {
	TRACE_CALL();
	if (!accept(parser, LexemType_WHILE)
		|| !expect(parser, LexemType_LEFT_BRACKET)) return NULL;

	Tree * exprNode = expr(parser);
	if (!exprNode) {
		return NULL;
	}
	if (!expect(parser, LexemType_RIGHT_BRACKET)) {  
		return NULL;
	}
	Tree * blockNode = block_actions(parser);
	if (blockNode == NULL) {
		return NULL;
	}

	Tree * whileNode = new Tree(new AstNode(AstNodeType_WHILE, "while"));

	whileNode->children.push_back(exprNode);
	whileNode->children.push_back(blockNode);
	return whileNode;
}

static Tree * switch_action(Parser * parser) {
	if (!accept(parser, LexemType_SWITCH)
		|| !expect(parser, LexemType_LEFT_BRACKET)) return NULL;

	Tree * exprNode = expr(parser);

	if (!exprNode) {
		return NULL;
	}
	if (!expect(parser, LexemType_RIGHT_BRACKET)) {
		return NULL;
	}
	if (!expect(parser, LexemType_LEFT_BRACE)) {
		return NULL;
	}

	Tree * switchNode = new Tree(new AstNode(AstNodeType_SWITCH, "switch"));
	switchNode->children.push_back(exprNode);

	if (!ebnf_sequence(parser, switchNode, switch_operator)) {
		return NULL;
	}

	if (accept(parser, LexemType_DEFAULT)) {
		if (!expect(parser, LexemType_COLON)) {
			return NULL;
		}
		Tree * default_node = new Tree(new AstNode(AstNodeType_DEFAULT, "default"));
		Tree * blockNode = action(parser);
		if (blockNode == NULL || !parser->error.empty()) {
			return NULL;
		}
		default_node->children.push_back(blockNode);
		switchNode->children.push_back(default_node);
	}

	if (!expect(parser, LexemType_RIGHT_BRACE)) {
		return NULL;
	}
	return switchNode;
}

static Tree * switch_operator(Parser * parser) {
	if (!accept(parser, LexemType_CASE)) return NULL;

	Tree * exprNode = expr(parser);

	if (!exprNode) {
		return NULL;
	}

	if (!expect(parser, LexemType_COLON)) {
		return NULL;
	}

	Tree * actionNode = action(parser);
	if (actionNode == NULL) {
		return NULL;
	}

	Tree * case_node = new Tree(new AstNode(AstNodeType_CASE, "case"));
	case_node->children.push_back(exprNode);
	case_node->children.push_back(actionNode);
	return case_node;
}

static Tree * print_action(Parser * parser) {
	if (!accept(parser, LexemType_PRINT)
		|| !expect(parser, LexemType_LEFT_BRACKET)) {
		return NULL;
	}

	Tree * exprNode = expr(parser);

	if (!exprNode) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACKET)
		|| !expect(parser, LexemType_SEMICOLON)) {
		return NULL;
	}

	Tree * printNode = new Tree(new AstNode(AstNodeType_PRINT, "print"));
	printNode->children.push_back(exprNode);
	return printNode;
}

static Tree * if_action(Parser * parser) {
	/*TRACE_CALL();
	return accept(parser, LexemType_IF)
		&& expect(parser, LexemType_LEFT_BRACKET)
		&& expr(parser)
		&& expect(parser, LexemType_RIGHT_BRACKET)
		&& block_actions(parser)
		&& (accept(parser, LexemType_ELSE) ? block_actions(parser) : true);*/
	TRACE_CALL();

	if (!accept(parser, LexemType_IF)
		|| !expect(parser, LexemType_LEFT_BRACKET)) return NULL;
	Tree * exprNode = expr(parser);

	if (!exprNode) {
		return NULL;
	}

	if (!expect(parser, LexemType_RIGHT_BRACKET)) {   
		return NULL;
	}

	Tree * actionNode = action(parser);

	if (actionNode == NULL) {
		return NULL;
	}
	Tree * ifNode = new Tree(new AstNode(AstNodeType_IF, "if"));

	ifNode->children.push_back(exprNode);
	ifNode->children.push_back(actionNode);

	if (accept(parser, LexemType_ELSE)) {

		Tree * elseNode = action(parser);
		if (elseNode == NULL || !parser->error.empty()) {
			return NULL;
		}

		ifNode->children.push_back(elseNode);
	}
	return ifNode;
}

/*
static bool cases_action(Parser * parser) {
	return accept(parser, LexemType_CASE)
		&& expr(parser)
		&& accept(parser, LexemType_OF)
		&&

}*/

static Tree * substitution_action(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_SUBSTITUTE)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_FOR)
		&& expect(parser, LexemType_NAME)
		&& expect(parser, LexemType_WHEN)
		&& expr(parser)
		&& expect(parser, LexemType_SEMICOLON);*/

	if (!accept(parser, LexemType_SUBSTITUTE)) return NULL;

	Tree * idNode1 = expect(parser,LexemType_NAME);
	if (!idNode1) return NULL;

	if (!expect(parser, LexemType_FOR)) return NULL;

	Tree * idNode2 = expect(parser, LexemType_NAME);
	if (!idNode2) return NULL;

	if (!expect(parser, LexemType_WHEN)) return NULL;

	Tree * exprNode = expr(parser);
	if (!exprNode) return NULL;

	if (!expect(parser, LexemType_SEMICOLON)) return NULL;

	Tree * substitutionNode = new Tree(new AstNode(AstNodeType_SUBSTITUTION, "substitution"));
	substitutionNode->children.push_back(idNode1);
	substitutionNode->children.push_back(idNode2);
	substitutionNode->children.push_back(exprNode);
	return substitutionNode;


}

static Tree * timeline_action(Parser * parser) {

	/*TRACE_CALL();
	return accept(parser, LexemType_TIMELINE)
		&& timeline_overload(parser)
		&& block_actions(parser);*/
	if (!accept(parser, LexemType_TIMELINE)) return NULL;

	Tree * exprNode = timeline_overload(parser);
	if (exprNode == NULL) {
		return NULL;
	}

	Tree * actionNode = block_actions(parser);
	if (actionNode == NULL) {
		return NULL;
	}

	Tree * timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE, "timeline"));
	timelineNode->children.push_back(exprNode);
	timelineNode->children.push_back(actionNode);
	return timelineNode;
}

static Tree * timeline_overload(Parser * parser) {
	TRACE_CALL();

	GrammarRule rules[] = {
			timeline_expr,
			timeline_as,
			timeline_until
	};
	return ebnf_one_of(parser, rules, 3);
}

static Tree * timeline_expr(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_LEFT_BRACKET)
		&& expr(parser)
		&& expect(parser, LexemType_COLON)
		&& expr(parser)
		&& expect(parser, LexemType_COLON)
		&& expr(parser)
		&& expect(parser, LexemType_RIGHT_BRACKET);*/
	if (!accept(parser, LexemType_LEFT_BRACKET)) return NULL;

	Tree * exprNode1 = expr(parser);
	if (exprNode1 == NULL) {
		return NULL;
	}
	if (!expect(parser, LexemType_COLON)) return NULL;

	Tree * exprNode2 = expr(parser);
	if (exprNode2 == NULL) {
		return NULL;
	}
	if (!expect(parser, LexemType_COLON)) return NULL;

	Tree * exprNode3 = expr(parser);
	if (exprNode3 == NULL) {
		return NULL;
	}
	if (!expect(parser, LexemType_RIGHT_BRACKET)) return NULL;

	Tree * timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE_EXPR, "timeline_expr"));
	timelineNode->children.push_back(exprNode1);
	timelineNode->children.push_back(exprNode2);
	timelineNode->children.push_back(exprNode3);
	return timelineNode;
}

static Tree * timeline_as(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_AS)
		&& expect(parser, LexemType_NAME);*/
	if (!accept(parser, LexemType_AS)) return NULL;

	Tree * idNode = ID(parser);
	if (idNode == NULL) {
		return NULL;
	}
	Tree * timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE_AS, "timeline_as"));
	timelineNode->children.push_back(idNode);
	return timelineNode;
}

static Tree * timeline_until(Parser * parser) {
	TRACE_CALL();
	/*return accept(parser, LexemType_UNTIL)
		&& expr(parser);*/
	if (!accept(parser, LexemType_UNTIL)) return NULL;

	Tree * exprNode = expr(parser);
	if (exprNode == NULL) {
		return NULL;
	}
	Tree * timelineNode = new Tree(new AstNode(AstNodeType_TIMELINE_UNTIL, "timeline_until"));
	timelineNode->children.push_back(exprNode);
	return timelineNode;
}



static Tree * expr(Parser * parser) {
	TRACE_CALL();
	return assign(parser);
}

static Tree * expr_st(Parser * parser) {
	/*TRACE_CALL();
	if (expr(parser)) {
		return expect(parser, LexemType_SEMICOLON);
	}
	return accept(parser, LexemType_SEMICOLON);*/
	TRACE_CALL();
	Tree * exprNode = expr(parser);

	if (exprNode) {
		expect(parser, LexemType_SEMICOLON);
	}else {
		accept(parser, LexemType_SEMICOLON);
	}
	return exprNode;
}

static Tree * assign(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, log_or, assign_ap);
}
static Tree * assign_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_ASSIGN
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), log_or, assign_ap);
}

static Tree * log_or(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, log_and, log_or_ap);
}

static Tree * log_or_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_OR
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), log_and, log_or_ap);
}

static Tree * log_and(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, eq, log_and_ap);
}

static Tree * log_and_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_AND
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), eq, log_and_ap);
}

static Tree * eq(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, rel, eq_ap);
}

static Tree * eq_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_EQUAL,
		LexemType_NOTEQUAL
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), rel, eq_ap);
}

static Tree * rel(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, add, rel_ap);
}

static Tree * rel_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_MORE,
		LexemType_LESS,
		LexemType_LESS_OR_EQUAL,
		LexemType_MORE_OR_EQUAL

	};

	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), add, rel_ap);
}

static Tree * add(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, mult, add_ap);
}
static Tree * add_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_PLUS,
		LexemType_MINUS
	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), mult, add_ap);
}

static Tree * mult(Parser * parser) {
	TRACE_CALL();
	return ebnf_ap_main_rule(parser, unary, mult_ap);
}
static Tree * mult_ap(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_MULT,
		LexemType_DIV,
		LexemType_MOD

	};
	return ebnf_ap_recursive_rule(parser, arr, (sizeof(arr) / sizeof(*arr)), unary, mult_ap);
}

static Tree * unary(Parser * parser) {
	TRACE_CALL();
	LexemType arr[] = {
		LexemType_PLUS,
		LexemType_MINUS,
		LexemType_NOT
	};
	Tree * opNode = ebnf_one_of_lexem(parser, arr, (sizeof(arr) / sizeof(*arr)));
	Tree * primNode = primary(parser);

	if (opNode) {
		opNode->children.push_back(primNode);
		return opNode;
	}
	return primNode;
}

static Tree * primary(Parser * parser) {
	TRACE_CALL();
	GrammarRule arr[] = {
		NUMBER,
		String,
		var_or_call,
		parentheses
	};
	return ebnf_one_of(parser, arr, (sizeof(arr) / sizeof(*arr)));
}

static Tree * var_or_call(Parser * parser) {
	TRACE_CALL();
	Tree * varNode = ID(parser);

	Tree * argListNode = fn_call(parser);

	if (argListNode) {
		varNode->children.push_back(argListNode);
	}
	return varNode;
}
static Tree * parentheses(Parser * parser) {
	TRACE_CALL();
	if (!accept(parser, LexemType_LEFT_BRACKET)) return NULL;
	Tree * exprNode = expr(parser);
	expect(parser, LexemType_RIGHT_BRACKET); // @todo mb 
	return exprNode;
}
static Tree * fn_call(Parser * parser) {
	if (!accept(parser, LexemType_LEFT_BRACKET)) return NULL;
	Tree * argListNode = arg_list(parser);
	expect(parser, LexemType_RIGHT_BRACKET); 
	return argListNode;
}

static Tree * arg_list(Parser * parser) {
	/*TRACE_CALL();
	return expr(parser)
		&& (accept(parser, LexemType_COMMA) ? arg_list(parser) : true);*/
	TRACE_CALL();
	Tree * exprNode = expr(parser);
	Tree * argListNode = new Tree(new AstNode(AstNodeType_ARGLIST, "arglist"));

	if (exprNode != NULL) {
		//List_add(argListNode->children, exprNode);
		argListNode->children.push_back(exprNode);
		while (true) {
			if (!accept(parser, LexemType_COMMA)) break;

			exprNode = expr(parser);
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

static Tree * data(Parser * parser) {
	TRACE_CALL();
	GrammarRule arr[] = {
		expr,
		arr_ini,
	};
	return ebnf_one_of(parser, arr, 2);
}

static Tree * arr_ini(Parser * parser) {
	TRACE_CALL();
	if (!accept(parser, LexemType_LEFT_BRACE)) return NULL;
	Tree * argListNode = arg_list(parser);
	expect(parser, LexemType_RIGHT_BRACE); 
}

static AstNodeType TokenType_toAstNodeType(LexemType type) {
	switch (type)
	{
	case LexemType_ASSIGN: return AstNodeType_ASSIGN;
	case LexemType_PLUS: return AstNodeType_ADD;
	case LexemType_MINUS: return AstNodeType_SUB;
	case LexemType_MULT: return AstNodeType_MUL;
	case LexemType_DIV: return AstNodeType_DIV;
	case LexemType_MOD: return AstNodeType_MOD;
	case LexemType_EQUAL: return AstNodeType_EQUAL;
	case LexemType_NOTEQUAL: return AstNodeType_NOTEQUAL;
	case LexemType_NOT: return AstNodeType_NOT;
	case LexemType_MORE: return AstNodeType_MORE;
	case LexemType_LESS: return AstNodeType_LESS;
	case LexemType_MORE_OR_EQUAL: return AstNodeType_MORE_OR_EQUAL;
	case LexemType_LESS_OR_EQUAL: return AstNodeType_LESS_OR_EQUAL;
	case LexemType_AND: return AstNodeType_AND;
	case LexemType_OR: return AstNodeType_OR;

	case LexemType_NAME: return AstNodeType_ID;
	case LexemType_NUMBER: return AstNodeType_NUMBER;
	case LexemType_STRING_LITERAL: return AstNodeType_STRING;
	case LexemType_LOGIC: return AstNodeType_BOOL;

	default:
		return AstNodeType_UNKNOWN;
	}
}