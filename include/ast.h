#pragma once

#include <string>

enum class AstNodeType {
	UNKNOWN,
	//
	ASSIGN,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	EQUAL,
	NOTEQUAL,
	NOT,
	MORE,
	LESS,
	MORE_OR_EQUAL,
	LESS_OR_EQUAL,
	AND,
	OR,
    //
    FUNCTION_CALL,
	//
	NUMBER,
	STRING,
	ID,
	BOOL,
	//
	ARGLIST,
	//
	TIMELINE_EXPR,
	TIMELINE_AS,
	TIMELINE_UNTIL,
	//
	BLOCK,
	IF,
	WHILE,

	SWITCH,
	CASE,
	DEFAULT,

	TIMELINE,
	SUBSTITUTION,
	SEQUENCE,
	DOWNLOAD,
	UPLOAD,
	RENDER,

	PRINT,
	//
	LIB_IMPORT,
	ITEM_IMPORT,
	ELEMENT_IMPORT,
	//
	LIBRARIES,
	HANDLERS,
	RENDERERS,
	SOURCES,
	SETS,
	ELEMENTS,
	TUPLES,
	AGGREGATES,
	ACTIONS,
	//
	PROGRAM,
};

class AstNode {
public:
	AstNode() :
		type_(AstNodeType::UNKNOWN),
		value_("")
	{}

	AstNode(AstNodeType type, std::string value) :
		type_(type),
		value_(value)
	{}
public:
	AstNodeType type_;
	std::string	value_;
};
