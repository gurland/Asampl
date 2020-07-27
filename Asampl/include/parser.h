#pragma once

#include <vector>
#include <functional>

#include "lexer.h"
#include "tree.h"

//using namespace Lexer;

class Parser;

using grammar_rule_t = std::function<Tree *()>;

using token_iterator_t = std::vector<Lexer::Token>::iterator;

class Parser {
public:
	Parser(std::vector<Lexer::Token> *token_sequence) :
		token_sequence_(token_sequence),
		iterator_(token_sequence->begin()),
		level_(-1)
	{}

	Tree *buid_tree();
private:
	void set_error(const std::string& err) { error_ = err; }
	const std::string& get_error() { return error_; }

	void increase_level() { ++level_; }
	void reduce_level() { --level_; }

	Lexer::Token get_iterator_value() { return *iterator_; }
	const token_iterator_t& get_iterator() { return iterator_; }

	void increase_iterator() { ++iterator_; };

	const std::vector<Lexer::Token>* const get_token_sequence() { return token_sequence_; };
private:
	Tree* id();
	Tree* string();
	Tree* boolean();
	Tree* number();

private:
	bool eoi();

	Tree *accept(Lexer::TokenType token);
	Tree *expect(Lexer::TokenType token);

	bool ebnf_sequence(Tree* node_to_fill, grammar_rule_t rule);
	Tree* ebnf_one_of(grammar_rule_t rules[], size_t length);
	Tree* ebnf_one_of_lexem(Lexer::TokenType types[], size_t length);
	Tree* ebnf_ap_main_rule(grammar_rule_t next, grammar_rule_t ap);
	Tree* ebnf_ap_recursive_rule(Lexer::TokenType types[], size_t typesLen, grammar_rule_t next, grammar_rule_t ap);

	Tree *program();

	Tree *libraries_section();
	Tree *library_import();

	Tree *handlers_section();
	Tree *renderers_section();


	Tree *sources_section();
	//bool source_declaration(Parser *parser);

	Tree *sets_section();

	Tree *item_import();


	Tree *elements_section();
	Tree *element_declaration();

	Tree *tuples_section();
	//bool tuples_declaration(Parser *parser);

	Tree *aggregates_section();
	//bool aggregates_declaration(Parser *parser);


	Tree *actions_section();
	Tree *action();

	Tree *block_actions();

	Tree *sequence_action();
	Tree *download_action();
	Tree *upload_action();
	Tree *render_action();
	Tree *if_action();
	Tree *while_action();
	Tree *switch_action();
	Tree *switch_operator();

	Tree *print_action();

	Tree *substitution_action();

	Tree *timeline_action();
	Tree *timeline_overload();

	Tree *timeline_expr();
	Tree *timeline_as();
	Tree *timeline_until();
	//bool timeline_while(Parser *parser);


	Tree *expr();
	Tree *expr_st();
	//bool block_st(Parser *parser);


	Tree *assign();
	Tree *assign_ap();
	Tree *log_or();
	Tree *log_or_ap();
	Tree *log_and();
	Tree *log_and_ap();
	Tree *eq();
	Tree *eq_ap();
	Tree *rel();
	Tree *rel_ap();
	Tree *add();
	Tree *add_ap();
	Tree *mult();
	Tree *mult_ap();
	Tree *unary();
	Tree *primary();
	Tree *var_or_call();
	Tree *parentheses();
	Tree *fn_call();
	Tree *arg_list();
	Tree *data();
	Tree *arr_ini();
	//

private:
	std::vector<Lexer::Token> *token_sequence_;
	token_iterator_t iterator_;
	std::string error_;
	int level_;
};

Tree *parser_buid_tree(std::vector<Lexer::Token> *lexem_sequence);

