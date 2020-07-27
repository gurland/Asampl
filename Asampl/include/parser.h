#pragma once

#include <vector>

#include "lexer.h"
#include "tree.h"

//using namespace Lexer;

using token_iterator_t = std::vector<Lexer::Token>::iterator;

class Parser {
public:
	Parser(std::vector<Lexer::Token> *token_sequence) :
		token_sequence_(token_sequence),
		iterator_(token_sequence->begin()),
		level_(-1)
	{}

	const void set_error(const std::string &err) { error_ = err; }
	const std::string &get_error() { return error_; }

	void increase_level() { ++level_; }
	void reduce_level() { --level_; }

	Lexer::Token get_iterator_value() { return *iterator_; }
	const token_iterator_t &get_iterator() { return iterator_; }

	void increase_iterator() { ++iterator_; };

	const std::vector<Lexer::Token> *const get_token_sequence() { return token_sequence_; };
	
	Tree *buid_tree();
private:
	std::vector<Lexer::Token> *token_sequence_;
	token_iterator_t iterator_;
	std::string error_;
	int level_;
};


