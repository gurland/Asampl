#pragma once
#include <vector>
#include <algorithm>

#include <iostream>
#include <string>

typedef enum {
	AstNodeType_UNKNOWN,
	//
	AstNodeType_ASSIGN,
	AstNodeType_ADD,
	AstNodeType_SUB,
	AstNodeType_MUL,
	AstNodeType_DIV,
	AstNodeType_MOD,
	AstNodeType_EQUAL,
	AstNodeType_NOTEQUAL,
	AstNodeType_NOT,
	AstNodeType_MORE,
	AstNodeType_LESS,
	AstNodeType_MORE_OR_EQUAL,
	AstNodeType_LESS_OR_EQUAL,
	AstNodeType_AND,
	AstNodeType_OR,
	//
	AstNodeType_NUMBER,
	AstNodeType_STRING,
	AstNodeType_ID,
	AstNodeType_BOOL,
	//
	AstNodeType_ARGLIST,
	//
	AstNodeType_TIMELINE_EXPR,
	AstNodeType_TIMELINE_AS,
	AstNodeType_TIMELINE_UNTIL,
	//
	AstNodeType_BLOCK,
	AstNodeType_IF,
	AstNodeType_WHILE,

	AstNodeType_SWITCH,
	AstNodeType_CASE,
	AstNodeType_DEFAULT,

	AstNodeType_TIMELINE,
	AstNodeType_SUBSTITUTION,
	AstNodeType_SEQUENCE,
	AstNodeType_DOWNLOAD,
	AstNodeType_UPLOAD,
	AstNodeType_RENDER,

	AstNodeType_PRINT,
	//
	AstNodeType_LIB_IMPORT,
	AstNodeType_ITEM_IMPORT,
	AstNodeType_ELEMENT_IMPORT,
	//
	AstNodeType_LIBRARIES,
	AstNodeType_HANDLERS,
	AstNodeType_RENDERERS,
	AstNodeType_SOURCES,
	AstNodeType_SETS,
	AstNodeType_ELEMENTS,
	AstNodeType_TUPLES,
	AstNodeType_AGGREGATES,
	AstNodeType_ACTIONS,
	//
	AstNodeType_PROGRAM,
} AstNodeType;


class AstNode{
public:
	AstNodeType type;
	std::string	value;

	AstNode() {

	}

	AstNode(AstNodeType type, std::string value) : type(type), value(value) {

	}
};

class Tree{
public:
	AstNode * value;
	std::vector<Tree*> children;

	Tree()
	{

	}

	Tree(AstNode * value) : value(value){

	}

	/*void print() {

		std::cout << value->name << std::endl;
		
		std::for_each(children.begin(),
			children.end(),
			[](void * child) {
				((Tree *)child)->print();
			}
		);
	}*/

	
	

};

/*TRACE_CALL();

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
}*/

/*TRACE_CALL();
    if (!bool_accept(parser, TokenType_LET)) return NULL;
    Tree * idNode = ID_expect(parser);
    if (!idNode) return NULL;

    if (!bool_expect(parser, TokenType_ASSIGN)) {
        Parser_releaseTheTree(idNode);
        // parser->error = strdup("ERROR");
        // @todo error
        return NULL;
    }

    // Tree * exprNode = expr(parser);
    Tree * dataNode = data(parser);
    if (dataNode == NULL) {
        Parser_releaseTheTree(idNode);
        Parser_releaseTheTree(dataNode);
        
        // @todo error
        return NULL;
    } 
    if (!bool_expect(parser, TokenType_SEMICOLON)) {
        Parser_releaseTheTree(idNode);
        Parser_releaseTheTree(dataNode);
        // @todo error
        return NULL;
    }

    Tree * varDecl = Tree_new(AstNode_new(AstNodeType_DECLAREVAR, "declareVar"));

    List_add(varDecl->children, idNode);
    List_add(varDecl->children, dataNode);
    return varDecl;*/