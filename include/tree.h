#pragma once
#include <vector>
#include <algorithm>

#include <memory>
#include <iostream>
#include <string>
#include <type_traits>

#include "ast.h"

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

    const Tree* find_child(AstNodeType type) const {
        for (auto& child : children_) {
            if (child->get_node()->type_ == type) {
                return child;
            }
        }

        return nullptr;
    }

    template< typename M, typename... Ms >
    bool match(M&& node_matcher, Ms&&... child_matchers) const;

    template< typename... Ms >
    bool match_children(Ms&&... child_matchers) const;

private:
	children_t children_;
	std::unique_ptr<AstNode> node_;
};

#include "matcher.h"

template< typename M, typename... Ms >
bool Tree::match(M&& node_matcher, Ms&&... child_matchers) const {
    if (!Matcher::match_one(*node_, std::forward<M>(node_matcher))) {
        return false;
    }
    return Matcher::match_impl(children_, 0, std::forward<Ms>(child_matchers)...);
}

template< typename... Ms >
bool Tree::match_children(Ms&&... child_matchers) const {
    return Matcher::match_impl(children_, 0, std::forward<Ms>(child_matchers)...);
}

#include "matcher_impl.h"
