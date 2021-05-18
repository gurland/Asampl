#ifndef _TREE_H
#define _TREE_H

#include <vector>
#include <algorithm>

#include <memory>
#include <iostream>
#include <string>
#include <type_traits>


enum class ast_node_type {
	PROGRAM,

	PARAM_LIST,
	ARG_LIST,

	FN_CALL,
	ARRAY,
	ARR_EL,

	BLOCK,
	IF,
	WHILE,
	MATCH,
	CASE,
	DEF_CASE,
	MATCH_LIST,

	OBJ_DECL,
	OBJ_FIELD,

	LAMBDA,

	HANDLER,
	IMPORT,
	FROM,
	ELSE,
	TIMELINE,
	DOWNLOAD,
	UPLOAD,
	TO,
	FN,
	LET,
	LOGIC,
	WITH,
	CONTINUE,
	BREAK,
	RETURN,

	NOT,
	BIN_NOT,

	ID,
	STRING,
	NUMBER,

	SEMICOLON,
	LEFT_BRACE,
	RIGHT_BRACE,
	LEFT_SQUARE_BRACKET,
	RIGHT_SQUARE_BRACKET,
	COMMA,
	DOT,
	COLON,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	EQUAL,
	NOT_EQUAL,
	LESS_EQUAL,
	MORE_EQUAL,

	ASSIGNMENT,
	DIV_ASSIGNMENT,
	PLUS_ASSIGNMENT,
	MINUS_ASSIGNMENT,
	MULT_ASSIGNMENT,
	MDIV_ASSIGNMENT,
	LEFT_SHIFT_ASSIGNMENT,
	RIGHT_SHIFT_ASSIGNMENT,
	BIN_AND_ASSIGNMENT,
	BIN_OR_ASSIGNMENT,
	BIN_NOR_ASSIGNMENT,

	DIV,
	PLUS,
	MINUS,
	MULT,
	MDIV,
	LESS,
	MORE,
	BIN_AND,
	BIN_OR,
	BIN_NOR,
	LOG_AND,
	LOG_OR,
	INCREM,
	DECREM,

	ARROW,
	LEFT_SHIFT_OPERATOR,
	RIGHT_SHIFT_OPERATOR,

	LEFT_SHIFT,
	RIGHT_SHIFT,
	QUESTION_MARK,
	FILE_NAME,

	NONE,
};

using ast_nt = ast_node_type;

class ast_node {
public:
	ast_node() :
		type(ast_nt::NONE),
		value((long)0)
	{}

	ast_node(ast_nt _type, std::string _value) :
		type(_type),
		value(_value)
	{}

	ast_node(ast_nt _type) :
		type(_type),
		value((long)0)
	{}

	ast_node(ast_nt _type, long _val) :
		type(_type),
		value(_val)
	{}

	ast_node(ast_nt _type, double _val) :
		type(_type),
		value(_val)
	{}

	ast_nt type;
    std::variant<long, std::string, double> value;
};

class as_tree;
using ast_children = std::vector<as_tree *>;

#include "vt.h"
extern std::string at_to_string(ast_nt type);

class as_tree {
public:

	as_tree()
	{}

	as_tree(ast_node *value) :
		node(value)
	{}

	void print(std::ostream &file, const std::string &indent = "", bool root = true, int last = 1) {
		if (root) file << "\n";
		file << indent;
		std::string new_indent = "";
		if (last) {
			if (!root) {
				file << "`-";
				new_indent = indent + "**";
			}
			else {
				new_indent = indent;
			}
		}
		else {
			file << "|-";
			new_indent = indent + "|*";
		}
		// if (get_vt(this->node, value) == vt::STRING && get_str_val(this->node, value) == "fn") {
		// 	int il = 0;
		// }
		file << ((get_vt(this->node, value) == vt::STRING) ?
			get_str_val(this->node, value) + "\n" :
			at_to_string(this->node->type) + "\n");
		size_t count = this->children.size();
		for (int i = 0; i < count; ++i) {
			this->children.at(i)->print(file, new_indent, false, i == count - 1);
		}
	}

	static void free(as_tree *tree) {
		if (!tree) return;
		ast_children children = tree->children;
		for (auto child : children) {
			as_tree::free(child);
		}
	}

	const ast_node *get_node() const { return node.get(); }
	const ast_children &get_children() const { return children; }

	void add_child(as_tree *child) {
		children.emplace_back(child);
	}
	void add_child(ast_children::const_iterator it, as_tree *child) {
		children.emplace(it, child);
	}

    const as_tree* find_child(ast_nt type) const {
        for (auto& child : children) {
            if (child->get_node()->type == type) {
                return child;
            }
        }

        return nullptr;
    }

    template< typename M, typename... Ms >
    bool match(M&& node_matcher, Ms&&... child_matchers) const;

    template< typename... Ms >
    bool match_children(Ms&&... child_matchers) const;

	ast_children children;
	std::unique_ptr<ast_node> node;
};

#include "matcher.h"

template< typename M, typename... Ms >
bool as_tree::match(M&& node_matcher, Ms&&... child_matchers) const {
    if (!Matcher::match_one(*node, std::forward<M>(node_matcher))) {
        return false;
    }
    return Matcher::match_impl(children, 0, std::forward<Ms>(child_matchers)...);
}

template< typename... Ms >
bool as_tree::match_children(Ms&&... child_matchers) const {
    return Matcher::match_impl(children, 0, std::forward<Ms>(child_matchers)...);
}

#include "matcher_impl.h"

#endif /* _TREE_H */
