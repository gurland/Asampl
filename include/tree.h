#ifndef _TREE_H
#define _TREE_H

#include <vector>
#include <algorithm>

#include <memory>
#include <iostream>
#include <string>
#include <type_traits>

enum class ast_node_type {
	UNKNOWN,

	PROGRAM,

	PARAM_LIST,
	ARG_LIST,

	BLOCK,
	IF,
	WHILE,
	MATCH,
	CASE,
	DEF_CASE,
	MATCH_LIST,

	TIMELINE,
	DOWNLOAD,
	UPLOAD,

	CONTINUE,
	BREAK,
	RETURN,

	OBJ_DECL,
	OBJ_FIELD,
};

using ast_nt = ast_node_type;

class ast_node {
public:
	ast_node() :
		type(ast_nt::UNKNOWN),
		value((int)0)
	{}

	ast_node(ast_nt _type, std::string _value) :
		type(_type),
		value(_value)
	{}

	ast_node(ast_nt _type) :
		type(_type),
		value((int)0)
	{}

	ast_node(ast_nt _type, int) :
		type(_type),
		value((int)0)
	{}

	ast_nt type;
    std::variant<int, std::string> value;
};

class as_tree;
using ast_children = std::vector<as_tree *>;

class as_tree {
public:

	as_tree()
	{}

	as_tree(ast_node *value) :
		node(value)
	{}

	void print(std::ostream &file, const std::string &indent = "", bool root = true, int last = 1);

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
