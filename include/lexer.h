#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#define IS_NAME_START(c) (isalpha( (c) ) || (c) == '_')
#define IS_NAME(c) ( IS_NAME_START( c ) || isdigit(c) )
#define IS_STRING_LITERAL_START(c) ((c) == '\'' || (c) == '\"')

namespace Lexer {
	enum class TokenType {
		//Base_Symbols
		INTEGER,
		REAL,

		NUMBER,

		STRING_LITERAL,
		NAME,

		//True/False
		LOGIC, //BOOLEAN

		//Math_Operators
		PLUS,
		MINUS,
		MULT,
		DIV,

		MOD, //May be

		CARET,

		//Logical_Operators
		AND,
		OR,
		NOT,
		XOR,

		EQUAL,
		NOTEQUAL,

		MORE,
		LESS,

		LESS_OR_EQUAL,
		MORE_OR_EQUAL,

		//Assign
		ASSIGN, //("=" or "IS")

		//Symbols
		POINT,
		COMMA,
		SEMICOLON,
		COLON,
		QUESTION_MARK,
		EXCLAMATION_MARK,

		LEFT_BRACKET,//"("
		RIGHT_BRACKET,//")"
		LEFT_BRACE, //"{"
		RIGHT_BRACE,//"}"
		LEFT_SQUARE_BRACKET,//"["
		RIGHT_SQUARE_BRACKET,//"]"

		APOSTROPHE,
		QUOTES, // "
		SLASH, // "/"
		BACKSLASH, // "\"
		NUMBER_SIGN, // "#"
		SOBAKA, //"@"

		AMPERSAND,
		VERTICAL_BAR,

		//Program key words
		PROGRAM,
		LIBRARIES,
		HANDLERS,
		RENDERERS,
		SOURCES,
		SETS,
		ELEMENTS,
		TUPLES,
		AGGREGATES,
		ACTIONS,

		//Special key words for

		//Timeline operator
		TIMELINE,
		AS,
		UNTIL,
		//Sequence handler operator
		SEQUENCE,

		//IF/Case/while
		IF,
		THEN,
		ELSE,
		SWITCH,
		DEFAULT,
		CASE,
		OF,
		WHILE,

		//SUBSTITUTE operator
		SUBSTITUTE,
		FOR,
		WHEN,

		//Download/Render operator
		DOWNLOAD,
		FROM,
		WITH,
		UPLOAD,
		TO,

		RENDER,
		PRINT,
		//TokenType_WITH,

		COMMENT,

		NOTHING,
	};


	class Token
	{
		std::string	buffer_;
		TokenType type_;
		int line_;

	public:
		Token() : type_(TokenType::NOTHING)
		{}

		Token(std::string buffer, TokenType type, int position_in_file) :
			buffer_(buffer), type_(type), line_(position_in_file)
		{}


		std::string get_buffer() const { return buffer_; }
		TokenType get_type() const { return type_; }
		int get_line() { return line_; }

		void set_buffer(std::string buffer) { buffer_ = buffer; }
		void set_type(TokenType type) { type_ = type; }
		void set_line(int line) { line_ = line; }

	};



	//Main function
	int split_tokens(std::fstream &fs, std::vector<Token> &token_sequence);

	//Print all
	void token_print(std::vector<Token> &token_sequence);

	std::string to_string(TokenType type);

}