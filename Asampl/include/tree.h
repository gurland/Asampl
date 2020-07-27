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

class Tree {
public:

	Tree()
	{}

	Tree(AstNode *value) :
		node_(value)
	{}

	void print(std::ostream &file, const std::string &indent = "", bool root = true, int last = 1) {
		if (root) file << "\n";
		file << indent;
		std::string new_indent = "";
		if (last) {
			if (!root) {
				file << "`-";
				new_indent = indent + "**";
			} else {
				new_indent = indent;
			}
		} else {
			file << "|-";
			new_indent = indent + "|*";
		}

		file << this->node_->value_ + "\n";
		size_t count = this->children_.size();
		for (int i = 0; i < count; ++i) {
			this->children_.at(i)->print(file, new_indent, false, i == count - 1);
		}
	}

	static void free(Tree *tree) {
		for (auto child : tree->children_) {
			Tree::free(child);
		}
		delete tree->node_;
	}

public:
	AstNode *node_;
	std::vector<Tree *> children_;
};
