#pragma once
#include <vector>
#include <algorithm>

#include <iostream>
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


class AstNode{

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

class Tree;
using children_t = std::vector<Tree *>;

class Tree {
public:

	Tree()
	{}

	Tree(AstNode *value) :
		node_(value)
	{}

	void print(std::ostream &file, const std::string &indent = "", bool root = true, int last = 1);

	static void free(Tree *tree) {
		children_t children = tree->children_;
		delete tree;
		for (auto child : children) {
			Tree::free(child);
		}
	}

	const AstNode *get_node() const { return node_.get(); }
	const children_t &get_children() const { return children_; }

	void add_child(Tree *child) {
		children_.emplace_back(child);
	}
	void add_child(children_t::const_iterator it, Tree *child) {
		children_.emplace(it, child);
	}
	
private:
	children_t children_;
	std::unique_ptr<AstNode> node_;
};
