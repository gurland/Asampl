#include "pch.h"
#include "lexer.h"

#include<string.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include <iostream>
#include <fstream>

#include <algorithm>

#include <string>
#include <map>

namespace Lexer {

	struct comp {
		bool operator() (const std::string &lhs, const std::string &rhs) const {
			return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};


	std::map <std::string, TokenType, comp> program_key_words = {

		{ "PROGRAM", TokenType::PROGRAM },
		{ "LIBRARIES", TokenType::LIBRARIES },
		{ "HANDLERS", TokenType::HANDLERS },
		{ "RENDERERS", TokenType::RENDERERS },
		{ "SOURCES", TokenType::SOURCES },
		{ "SETS", TokenType::SETS },
		{ "ELEMENTS", TokenType::ELEMENTS },
		{ "TUPLES", TokenType::TUPLES },
		{ "AGGREGATES", TokenType::AGGREGATES },
		{ "ACTIONS", TokenType::ACTIONS },

		{ "TIMELINE", TokenType::TIMELINE },
		{ "AS", TokenType::AS },
		{ "UNTIL", TokenType::UNTIL },
		{ "SEQUENCE", TokenType::SEQUENCE },

		{ "IF", TokenType::IF },
		{ "THEN", TokenType::THEN },
		{ "ELSE", TokenType::ELSE },
		{ "SWITCH", TokenType::SWITCH },
		{ "DEFAULT", TokenType::DEFAULT },
		{ "CASE", TokenType::CASE },
		{ "OF", TokenType::OF },
		{ "WHILE", TokenType::WHILE },

		{ "SUBSTITUTE", TokenType::SUBSTITUTE },
		{ "FOR", TokenType::FOR },
		{ "WHEN", TokenType::WHEN },

		{ "DOWNLOAD", TokenType::DOWNLOAD },
		{ "FROM", TokenType::FROM },
		{ "WITH", TokenType::WITH },
		{ "UPLOAD", TokenType::UPLOAD },
		{ "TO", TokenType::TO },

		{ "RENDER", TokenType::RENDER },

		{ "PRINT", TokenType::PRINT },

		{ "AND", TokenType::AND },
		{ "OR", TokenType::OR },
		{ "XOR", TokenType::NOT },
		{ "NOT", TokenType::XOR },

		{ "IS", TokenType::ASSIGN },

		{ "TRUE", TokenType::LOGIC },
		{ "FALSE", TokenType::LOGIC },

	};

	int current_line_position = 0;



	inline static int token_operator(std::fstream &fs, Token *token_to_add)
	{

		if (fs.peek() == '-')
		{
			token_to_add->set_buffer("-");
			token_to_add->set_type(TokenType::MINUS);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '+')
		{
			token_to_add->set_buffer("+");
			token_to_add->set_type(TokenType::PLUS);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '*')
		{
			token_to_add->set_buffer("*");
			token_to_add->set_type(TokenType::MULT);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '/')
		{
			fs.get();
			if (fs.peek() == '/') {
				std::string comment = "";
				while (fs.peek() != '\n') {
					comment += fs.get();;
					token_to_add->set_buffer(comment);
					token_to_add->set_type(TokenType::COMMENT);
					token_to_add->set_line(current_line_position);
				}
				return 0;
			}
			else {
				token_to_add->set_buffer("/");
				token_to_add->set_type(TokenType::DIV);
				token_to_add->set_line(current_line_position);
				return 0;
			}

		}

		else if (fs.peek() == '%')
		{
			token_to_add->set_buffer("%");
			token_to_add->set_type(TokenType::MOD);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '<')
		{
			fs.get();
			if (fs.peek() == '=') {
				token_to_add->set_buffer("<=");
				token_to_add->set_type(TokenType::LESS_OR_EQUAL);
				token_to_add->set_line(current_line_position);
				fs.get();
				return 0;
			}
			else {
				token_to_add->set_buffer("<");
				token_to_add->set_type(TokenType::LESS);
				token_to_add->set_line(current_line_position);
				return 0;
			}
		}

		else if (fs.peek() == '>')
		{
			fs.get();
			if (fs.peek() == '=') {
				token_to_add->set_buffer(">=");
				token_to_add->set_type(TokenType::MORE_OR_EQUAL);
				token_to_add->set_line(current_line_position);
				fs.get();
				return 0;
			}
			else {
				token_to_add->set_buffer(">");
				token_to_add->set_type(TokenType::MORE);
				token_to_add->set_line(current_line_position);
				return 0;
			}
		}

		else if (fs.peek() == '=')
		{
			fs.get();
			if (fs.peek() == '=') {
				token_to_add->set_buffer("==");
				token_to_add->set_type(TokenType::EQUAL);
				token_to_add->set_line(current_line_position);
				fs.get();
				return 0;
			}
			else {
				token_to_add->set_buffer("=");
				token_to_add->set_type(TokenType::ASSIGN);
				token_to_add->set_line(current_line_position);
				return 0;
			}
		}

		else if (fs.peek() == '!')
		{
			fs.get();
			if (fs.peek() == '=') {
				token_to_add->set_buffer("!=");
				token_to_add->set_type(TokenType::NOTEQUAL);
				token_to_add->set_line(current_line_position);
				fs.get();
				return 0;
			}
			else {
				token_to_add->set_buffer("!");
				token_to_add->set_type(TokenType::NOT);
				token_to_add->set_line(current_line_position);
				return 0;
			}
		}

		else if (fs.peek() == '?')
		{
			token_to_add->set_buffer("?");
			token_to_add->set_type(TokenType::QUESTION_MARK);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '^')
		{
			token_to_add->set_buffer("^");
			token_to_add->set_type(TokenType::CARET);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '&')
		{
			fs.get();
			if (fs.peek() == '&') {
				token_to_add->set_buffer("&&");
				token_to_add->set_type(TokenType::AND);
				token_to_add->set_line(current_line_position);
				fs.get();
				return 0;
			}
			else {
				token_to_add->set_buffer("&");
				token_to_add->set_type(TokenType::AMPERSAND);
				token_to_add->set_line(current_line_position);
				return 0;
			}
		}

		else if (fs.peek() == '|')
		{
			fs.get();
			if (fs.peek() == '|') {
				token_to_add->set_buffer("||");
				token_to_add->set_type(TokenType::OR);
				token_to_add->set_line(current_line_position);
				fs.get();
				return 0;
			}
			else {
				token_to_add->set_buffer("|");
				token_to_add->set_type(TokenType::VERTICAL_BAR);
				token_to_add->set_line(current_line_position);
				return 0;
			}
		}

		else if (fs.peek() == ':')
		{
			token_to_add->set_buffer(":");
			token_to_add->set_type(TokenType::COLON);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == ';')
		{
			token_to_add->set_buffer(";");
			token_to_add->set_type(TokenType::SEMICOLON);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '.')
		{
			token_to_add->set_buffer(".");
			token_to_add->set_type(TokenType::POINT);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == ',')
		{
			token_to_add->set_buffer(",");
			token_to_add->set_type(TokenType::COMMA);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '#')
		{
			token_to_add->set_buffer("#");
			token_to_add->set_type(TokenType::NUMBER_SIGN);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;

		}

		else if (fs.peek() == '@')
		{
			token_to_add->set_buffer("-");
			token_to_add->set_type(TokenType::SOBAKA);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}

		else if (fs.peek() == '(')
		{
			token_to_add->set_buffer("(");
			token_to_add->set_type(TokenType::LEFT_BRACKET);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == ')')
		{
			token_to_add->set_buffer(")");
			token_to_add->set_type(TokenType::RIGHT_BRACKET);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == '{')
		{
			token_to_add->set_buffer("{");
			token_to_add->set_type(TokenType::LEFT_BRACE);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == '}')
		{
			token_to_add->set_buffer("}");
			token_to_add->set_type(TokenType::RIGHT_BRACE);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == '[')
		{
			token_to_add->set_buffer("[");
			token_to_add->set_type(TokenType::LEFT_SQUARE_BRACKET);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == ']')
		{
			token_to_add->set_buffer("]");
			token_to_add->set_type(TokenType::RIGHT_SQUARE_BRACKET);
			token_to_add->set_line(current_line_position);
			fs.get();
			return 0;
		}
		else if (fs.peek() == EOF)
		{
			return 1;
		}

		else
		{
			token_to_add->set_buffer("ERROR");
			token_to_add->set_type(TokenType::NOTHING);
			token_to_add->set_line(current_line_position);
			fs.get();
			return -1;
		}
	}

	inline static int token_string_literal(std::fstream &fs, Token *token_to_add) {

		std::string buffer;

		if (fs.peek() == '\'') {
			fs.get();

			while (fs.peek() != '\'') {
				if (fs.peek() == '\n' || fs.peek() == EOF) break;

				buffer += (char)fs.get();
			}
		}
		else {
			while (fs.peek() != '\"') {
				if (fs.peek() == '\n' || fs.peek() == EOF) break;

				buffer += (char)fs.get();
			}
		}

		fs.get();

		token_to_add->set_buffer(buffer);
		token_to_add->set_type(TokenType::STRING_LITERAL);
		token_to_add->set_line(current_line_position);
		return 0;
	}

	// выделить лексему 'число'
	inline static int token_digit(std::fstream &fs, Token *token_to_add)
	{
		std::string buffer;

		buffer.clear();
		//int n = 0;

		int state = 0;

		while (fs.peek() != EOF) {

			switch (state)
			{
			case 0:

				if (isdigit(fs.peek())) {
					buffer += (char)fs.get();
				}
				else if (fs.peek() == '.') {
					buffer += (char)fs.get();
					state = 0;
				}
				else {
					token_to_add->set_buffer(buffer);
					token_to_add->set_type(TokenType::NUMBER);
					token_to_add->set_line(current_line_position);
					return 1;
				}
				break;

			case 1:

				if (isdigit(fs.peek())) {
					buffer += (char)fs.get();
				}
				else {
					token_to_add->set_buffer(buffer);
					token_to_add->set_type(TokenType::NUMBER);
					token_to_add->set_line(current_line_position);
					return 0;
				}
				break;
			}


		}

		return 0;

	}

	inline static int token_name(std::fstream &fs, Token *token_to_add)
	{

		std::string buffer;
		//int n = 0;

		while (fs.peek() != EOF) {
			if (!IS_NAME(fs.peek())) {
				break;
			}
			else {
				buffer += (char)fs.get();
			}
		}

		token_to_add->set_buffer(buffer);
		token_to_add->set_line(current_line_position);

		auto Lexem = program_key_words.find(buffer);

		if (Lexem != program_key_words.end()) {
			token_to_add->set_type(Lexem->second);
		}
		else {
			token_to_add->set_type(TokenType::NAME);
		}

		return 0;
	}


	//Ignore TAB, new lines, spaces
	inline static int skip_spaces(std::fstream &fs) {

		while (isspace(fs.peek()) || fs.peek() == '\n' || fs.peek() == '\t') {
			if (fs.peek() == '\n') {
				current_line_position++;
			}
			fs.get();
		}
		return 0;
	}


	int split_tokens(std::fstream &fs, std::vector<Token> &token_sequence) {

		if (!fs.is_open()) return 1;

		skip_spaces(fs);
		while (fs.peek() != EOF) {

			Token *token_to_add = new Token();

			if (IS_NAME_START(fs.peek())) {
				token_name(fs, token_to_add);
			} else if (isdigit(fs.peek())) {
				token_digit(fs, token_to_add);
			} else if (IS_STRING_LITERAL_START(fs.peek())) {
				token_string_literal(fs, token_to_add);
			} else {
				token_operator(fs, token_to_add);
			}

			if (token_to_add->get_type() == TokenType::NOTHING) {
				return -1;
			}

			token_sequence.push_back(*token_to_add);
			skip_spaces(fs);
		}
		return 0;

	}

	void token_print(std::vector<Token> &token_sequence)
	{
		for (const auto& token : token_sequence) {
			std::string str = "\t\tValue: ";
			if ((token.get_buffer().length() <= 8)) {
				str = "\t\t\tValue: ";
			}
			else if ((token.get_buffer().length() > 16)) {
				str = "\tValue: ";
			}
			std::cout <<
				"Lexem: " << token.get_buffer() <<
				str << to_string(token.get_type()) << "\n";
		}
	}

	std::string to_string(TokenType type) {
		switch (type) {
			//Base_Symbols
		case TokenType::INTEGER: return "INTEGER";
		case TokenType::REAL: return "REAL";
		case TokenType::NUMBER: return "NUMBER";


		case TokenType::STRING_LITERAL: return "STRING_LITERAL";
		case TokenType::NAME: return "NAME";

			//True/False
		case TokenType::LOGIC: return "LOGIC"; //BOOLEAN

			//Math_Operators
		case TokenType::PLUS: return "PLUS";
		case TokenType::MINUS: return "MINUS";
		case TokenType::MULT: return "MULT";
		case TokenType::DIV: return "DIV";

		case TokenType::MOD: return "MOD"; //May be

		case TokenType::CARET: return "CARET";

			//Logical_Operators
		case TokenType::AND: return "AND";
		case TokenType::OR: return "OR";
		case TokenType::NOT: return "NOT";
		case TokenType::XOR: return "XOR";

		case TokenType::EQUAL: return "EQUAL";
		case TokenType::NOTEQUAL: return "NOTEQUAL";

		case TokenType::MORE: return "MORE";
		case TokenType::LESS: return "LESS";

		case TokenType::LESS_OR_EQUAL: return "LESS_OR_EQUAL";
		case TokenType::MORE_OR_EQUAL: return "MORE_OR_EQUAL";

			//Assign
		case TokenType::ASSIGN: return "ASSIGN"; //("=" or "IS")

			//Symbols
		case TokenType::POINT: return "POINT";
		case TokenType::COMMA: return "COMMA";
		case TokenType::SEMICOLON: return "SEMICOLON";
		case TokenType::COLON: return "COLON";
		case TokenType::QUESTION_MARK: return "QUESTION_MARK";
		case TokenType::EXCLAMATION_MARK: return "EXCLAMATION_MARK";

		case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";//"("
		case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";//")"
		case TokenType::LEFT_BRACE: return "LEFT_BRACE"; //"{"
		case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";//"}"
		case TokenType::LEFT_SQUARE_BRACKET: return "LEFT_SQUARE_BRACKET";//"["
		case TokenType::RIGHT_SQUARE_BRACKET: return "RIGHT_SQUARE_BRACKET";//"]"

		case TokenType::APOSTROPHE: return "APOSTROPHE";
		case TokenType::QUOTES: return "QUOTES"; // "
		case TokenType::SLASH: return "SLASH"; // "/"
		case TokenType::BACKSLASH: return "BACKSLASH"; // "\"
		case TokenType::NUMBER_SIGN: return "NUMBER_SIGN"; // "#"
		case TokenType::SOBAKA: return "SOBAKA"; //"@"

		case TokenType::AMPERSAND: return "AMPERSAND";
		case TokenType::VERTICAL_BAR: return "VERTICAL_BAR";

			//Program key words
		case TokenType::PROGRAM: return "PROGRAM";
		case TokenType::LIBRARIES: return "LIBRARIES";
		case TokenType::HANDLERS: return "HANDLERS";
		case TokenType::RENDERERS: return "RENDERERS";
		case TokenType::SOURCES: return "SOURCES";
		case TokenType::SETS: return "SETS";
		case TokenType::ELEMENTS: return "ELEMENTS";
		case TokenType::TUPLES: return "TUPLES";
		case TokenType::AGGREGATES: return "AGGREGATES";
		case TokenType::ACTIONS: return "ACTIONS";

			//Special key words for

			//Timeline operator
		case TokenType::TIMELINE: return "TIMELINE";
		case TokenType::AS: return "AS";
		case TokenType::UNTIL: return "UNTIL";
			//Sequence handler operator
		case TokenType::SEQUENCE: return "SEQUENCE";

			//IF/Case
		case TokenType::IF: return "IF";
		case TokenType::THEN: return "THEN";
		case TokenType::ELSE: return "ELSE";
		case TokenType::CASE: return "CASE";
		case TokenType::OF: return "OF";

			//SUBSTITUTE operator
		case TokenType::SUBSTITUTE: return "SUBSTITUTE";
		case TokenType::FOR: return "FOR";
		case TokenType::WHEN: return "WHEN";

			//Download/Render operator
		case TokenType::DOWNLOAD: return "DOWNLOAD";
		case TokenType::FROM: return "FROM";
		case TokenType::WITH: return "WITH";
		case TokenType::UPLOAD: return "UPLOAD";
		case TokenType::TO: return "TO";

		case TokenType::RENDER: return "RENDER";
			//TokenType_WITH,

		case TokenType::COMMENT: return "COMMENT";

		case TokenType::NOTHING: return "NOTHING";
		default: return "";
		}
	}

}
