#ifndef _PARSER_H
#define _PARSER_H
#include <vector>

#include "lexer.h"
#include "tree.h"

// using token_cit = std::vector<token>::const_iterator;

// class parser {
// public:
// 	parser(std::vector<token> *token_sequence) :
// 		token_sequence(token_sequence),
// 		it(token_sequence->begin()),
// 		level(-1)
// 	{}

// 	const void set_error(const std::string &err) { error = err; }
// 	const std::string &get_error() { return error; }

// 	void increase_level() { ++level; }
// 	void reduce_level() { --level; }

// 	token get_it_value() { return *it; }
// 	const token_it &get_it() { return it; }

// 	void inc_it() { ++it; };

// 	const std::vector<token> *const get_token_sequence() { return token_sequence; };
	
// 	as_tree *buid_tree();
// private:
// 	std::vector<token> *token_sequence;
// 	token_cit it;
// 	// std::string error;
// 	int level;
// };

as_tree *buid_tree(std::vector<token> &token_sequence);
void release_tree(as_tree *tree);
std::string at_to_string(ast_nt type);
#endif /* _PARSER_H */