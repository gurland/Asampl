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

struct comp {
	bool operator() (const std::string& lhs, const std::string& rhs) const {
		return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
	}
};


std::map <std::string, LexemType, comp> program_key_words = {
	
	{ "PROGRAM", LexemType_PROGRAM },
	{ "LIBRARIES", LexemType_LIBRARIES },
	{ "HANDLERS", LexemType_HANDLERS },
	{ "RENDERERS", LexemType_RENDERERS },
	{ "SOURCES", LexemType_SOURCES },
	{ "SETS", LexemType_SETS },
	{ "ELEMENTS", LexemType_ELEMENTS },
	{ "TUPLES", LexemType_TUPLES },
	{ "AGGREGATES", LexemType_AGGREGATES },
	{ "ACTIONS", LexemType_ACTIONS },

	{ "TIMELINE", LexemType_TIMELINE },
	{ "AS", LexemType_AS },
	{ "UNTIL", LexemType_UNTIL },
	{ "SEQUENCE", LexemType_SEQUENCE },

	{ "IF", LexemType_IF },
	{ "THEN", LexemType_THEN },
	{ "ELSE", LexemType_ELSE },
	{ "SWITCH", LexemType_SWITCH },
	{ "DEFAULT", LexemType_DEFAULT },
	{ "CASE", LexemType_CASE },
	{ "OF", LexemType_OF },
	{ "WHILE", LexemType_WHILE },

	{ "SUBSTITUTE", LexemType_SUBSTITUTE },
	{ "FOR", LexemType_FOR },
	{ "WHEN", LexemType_WHEN },

	{ "DOWNLOAD", LexemType_DOWNLOAD },
	{ "FROM", LexemType_FROM },
	{ "WITH", LexemType_WITH },
	{ "UPLOAD", LexemType_UPLOAD },
	{ "TO", LexemType_TO },

	{ "RENDER", LexemType_RENDER },

	{ "PRINT", LexemType_PRINT },

	{ "AND", LexemType_AND },
	{ "OR", LexemType_OR },
	{ "XOR", LexemType_NOT },
	{ "NOT", LexemType_XOR },

	{ "IS", LexemType_ASSIGN },

	{ "TRUE", LexemType_LOGIC },
	{ "FALSE", LexemType_LOGIC },

};

int current_line_position = 0;



inline static int LexemOperator(std::fstream *fs, Lexem *lexem_to_add)
{

	if (fs->peek() == '-')
	{
		lexem_to_add->SetBuffer("-");
		lexem_to_add->SetType(LexemType_MINUS);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '+')
	{
		lexem_to_add->SetBuffer("+");
		lexem_to_add->SetType(LexemType_PLUS);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '*')
	{
		lexem_to_add->SetBuffer("*");
		lexem_to_add->SetType(LexemType_MULT);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '/')
	{
		fs->get();
		if (fs->peek() == '/') {
			std::string comment = "";
			while (fs->peek() != '\n') {
				comment += fs->get();;
				lexem_to_add->SetBuffer(comment);
				lexem_to_add->SetType(LexemType_COMMENT);
				lexem_to_add->SetLine(current_line_position);
			}
			return 0;
		}
		else {
			lexem_to_add->SetBuffer("/");
			lexem_to_add->SetType(LexemType_DIV);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}

	}

	else if (fs->peek() == '%')
	{
		lexem_to_add->SetBuffer("%");
		lexem_to_add->SetType(LexemType_MOD);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '<')
	{
		fs->get();
		if (fs->peek() == '=') {
			lexem_to_add->SetBuffer("<=");
			lexem_to_add->SetType(LexemType_LESS_OR_EQUAL);
			lexem_to_add->SetLine(current_line_position);
			fs->get();
			return 0;
		}
		else {
			lexem_to_add->SetBuffer("<");
			lexem_to_add->SetType(LexemType_LESS);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}
	}

	else if (fs->peek() == '>')
	{
		fs->get();
		if (fs->peek() == '=') {
			lexem_to_add->SetBuffer(">=");
			lexem_to_add->SetType(LexemType_MORE_OR_EQUAL);
			lexem_to_add->SetLine(current_line_position);
			fs->get();
			return 0;
		}
		else {
			lexem_to_add->SetBuffer(">");
			lexem_to_add->SetType(LexemType_MORE);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}
	}

	else if (fs->peek() == '=')
	{
		fs->get();
		if (fs->peek() == '=') {
			lexem_to_add->SetBuffer("==");
			lexem_to_add->SetType(LexemType_EQUAL);
			lexem_to_add->SetLine(current_line_position);
			fs->get();
			return 0;
		}
		else {
			lexem_to_add->SetBuffer("=");
			lexem_to_add->SetType(LexemType_ASSIGN);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}
	}

	else if (fs->peek() == '!')
	{
		fs->get();
		if (fs->peek() == '=') {
			lexem_to_add->SetBuffer("!=");
			lexem_to_add->SetType(LexemType_NOTEQUAL);
			lexem_to_add->SetLine(current_line_position);
			fs->get();
			return 0;
		}
		else {
			lexem_to_add->SetBuffer("!");
			lexem_to_add->SetType(LexemType_NOT);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}
	}

	else if (fs->peek() == '?')
	{
		lexem_to_add->SetBuffer("?");
		lexem_to_add->SetType(LexemTupe_QUESTION_MARK);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '^')
	{
		lexem_to_add->SetBuffer("^");
		lexem_to_add->SetType(LexemType_CARET);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '&')
	{
		fs->get();
		if (fs->peek() == '&') {
			lexem_to_add->SetBuffer("&&");
			lexem_to_add->SetType(LexemType_AND);
			lexem_to_add->SetLine(current_line_position);
			fs->get();
			return 0;
		}
		else {
			lexem_to_add->SetBuffer("&");
			lexem_to_add->SetType(LexemType_AMPERSAND);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}
	}

	else if (fs->peek() == '|')
	{
		fs->get();
		if (fs->peek() == '|') {
			lexem_to_add->SetBuffer("||");
			lexem_to_add->SetType(LexemType_OR);
			lexem_to_add->SetLine(current_line_position);
			fs->get();
			return 0;
		}
		else {
			lexem_to_add->SetBuffer("|");
			lexem_to_add->SetType(LexemType_VERTICAL_BAR);
			lexem_to_add->SetLine(current_line_position);
			return 0;
		}
	}

	else if (fs->peek() == ':')
	{
		lexem_to_add->SetBuffer(":");
		lexem_to_add->SetType(LexemType_COLON);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == ';')
	{
		lexem_to_add->SetBuffer(";");
		lexem_to_add->SetType(LexemType_SEMICOLON);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '.')
	{
		lexem_to_add->SetBuffer(".");
		lexem_to_add->SetType(LexemType_POINT);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == ',')
	{
		lexem_to_add->SetBuffer(",");
		lexem_to_add->SetType(LexemType_COMMA);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '#')
	{
		lexem_to_add->SetBuffer("#");
		lexem_to_add->SetType(LexemType_NUMBER_SIGN);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;

	}

	else if (fs->peek() == '@')
	{
		lexem_to_add->SetBuffer("-");
		lexem_to_add->SetType(LexemType_SOBAKA);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}

	else if (fs->peek() == '(')
	{
		lexem_to_add->SetBuffer("(");
		lexem_to_add->SetType(LexemType_LEFT_BRACKET);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == ')')
	{
		lexem_to_add->SetBuffer(")");
		lexem_to_add->SetType(LexemType_RIGHT_BRACKET);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == '{')
	{
		lexem_to_add->SetBuffer("{");
		lexem_to_add->SetType(LexemType_LEFT_BRACE);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == '}')
	{
		lexem_to_add->SetBuffer("}");
		lexem_to_add->SetType(LexemType_RIGHT_BRACE);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == '[')
	{
		lexem_to_add->SetBuffer("[");
		lexem_to_add->SetType(LexemType_LEFT_SQUARE_BRACKET);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == ']')
	{
		lexem_to_add->SetBuffer("]");
		lexem_to_add->SetType(LexemType_RIGHT_SQUARE_BRACKET);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return 0;
	}
	else if (fs->peek() == EOF)
	{
		return 1;
	}

	else
	{
		lexem_to_add->SetBuffer("ERROR");
		lexem_to_add->SetType(LexemType_NOTHING);
		lexem_to_add->SetLine(current_line_position);
		fs->get();
		return -1;
	}
}

inline static int LexemStringLiteral(std::fstream *fs, Lexem *lexem_to_add) {

	std::string buffer;

	if (fs->peek() == '\'') {
		fs->get();

		while (fs->peek() != '\'') {
			if (fs->peek() == '\n' || fs->peek() == EOF) break;

			buffer += (char)fs->get();
		}
	}
	else {
		while (fs->peek() != '\"') {
			if (fs->peek() == '\n' || fs->peek() == EOF) break;

			buffer += (char)fs->get();
		}
	}

	fs->get();

	lexem_to_add->SetBuffer(buffer);
	lexem_to_add->SetType(LexemType_STRING_LITERAL);
	lexem_to_add->SetLine(current_line_position);
	return 0;
}

// выделить лексему 'число'
inline static int LexemDigit(std::fstream *fs, Lexem *lexem_to_add)
{
	std::string buffer;

	buffer.clear();
	//int n = 0;

	int state = 0;

	while (fs->peek() != EOF) {

		switch (state)
		{
		case 0:

			if (isdigit(fs->peek())) {
				buffer += (char)fs->get();
			}
			else if (fs->peek() == '.') {
				buffer += (char)fs->get();
				state = 0;
			}
			else {
				lexem_to_add->SetBuffer(buffer);
				lexem_to_add->SetType(LexemType_NUMBER);
				lexem_to_add->SetLine(current_line_position);
				return 1;
			}
			break;

		case 1:

			if (isdigit(fs->peek())) {
				buffer += (char)fs->get();
			}
			else {
				lexem_to_add->SetBuffer(buffer);
				lexem_to_add->SetType(LexemType_NUMBER);
				lexem_to_add->SetLine(current_line_position);
				return 0;
			}
			break;
		}
		

	}

	return 0;
		
}

inline static int LexemName(std::fstream *fs, Lexem *lexem_to_add)
{

	std::string buffer;
	//int n = 0;

	while (fs->peek() != EOF) {
		if (!IS_NAME(fs->peek())) {
			break;
		}
		else {
			buffer += (char)fs->get();
		}
	}

	lexem_to_add->SetBuffer(buffer);
	lexem_to_add->SetLine(current_line_position);

	auto Lexem = program_key_words.find(buffer);

	if (Lexem != program_key_words.end()) {
		lexem_to_add->SetType(Lexem->second);
	}
	else {
		lexem_to_add->SetType(LexemType_NAME);
	}

	return 0;	
}


//Ignore TAB, new lines, spaces
inline static int  IgnoreTabNewlinesAndSpaces(std::fstream *fs) {

	while (isspace(fs->peek()) || fs->peek() == '\n' || fs->peek() == '\t') {
		if (fs->peek() == '\n') {
			current_line_position++;
		}
		fs->get();
	}
	return 0;
}


int Lexer_splitTokens(std::fstream *fs, std::vector<Lexem>* lexem_sequence) {

	if (fs == NULL) return 1;

	while (fs->peek() != EOF) {

		Lexem *lexem_to_add = new Lexem();

		IgnoreTabNewlinesAndSpaces(fs);


		if (IS_NAME_START(fs->peek()))
		{
			LexemName(fs, lexem_to_add);
		}

		else if (isdigit(fs->peek()))
		{
			LexemDigit(fs, lexem_to_add);
		}

		else if (IS_STRING_LITERAL_START(fs->peek()))
		{
			LexemStringLiteral(fs, lexem_to_add);
		}

		else {
			LexemOperator(fs, lexem_to_add);
		}

		if (lexem_to_add->GetType() == LexemType_NOTHING) {
			return -1;
		}
		
		lexem_sequence->push_back(*lexem_to_add);

	}
	return 0;

}

void LexemPrint(std::vector<Lexem>* lexem_sequence)
{

	std::for_each(lexem_sequence->begin(),
		lexem_sequence->end(),
		[&](Lexem lexem) {

		std::cout << " Lexem: " << lexem.GetBuffer() << "	Value: " << LexemType_toString(lexem.GetType()) << std::endl;

		}
	);
}

 std::string LexemType_toString(LexemType type) {
	switch (type) {
		//Base_Symbols
	case LexemType_INTEGER: return "LexemType_INTEGER";
	case	LexemType_REAL: return "LexemType_REAL";
	case LexemType_NUMBER: return "LexemType_NUMBER";
	

		case		LexemType_STRING_LITERAL: return "LexemType_STRING_LITERAL";
		case		LexemType_NAME: return "LexemType_NAME";

			//True/False
		case	LexemType_LOGIC: return "LexemType_LOGIC"; //BOOLEAN

			//Math_Operators
		case	LexemType_PLUS: return "LexemType_PLUS";
		case	LexemType_MINUS: return "LexemType_MINUS";
		case	LexemType_MULT: return "LexemType_MULT";
		case	LexemType_DIV: return "LexemType_DIV";

		case	LexemType_MOD: return "LexemType_MOD"; //May be

		case	LexemType_CARET: return "LexemType_CARET";

			//Logical_Operators
		case	LexemType_AND: return "LexemType_AND";
		case	LexemType_OR: return "LexemType_OR";
		case	LexemType_NOT: return "LexemType_NOT";
		case	LexemType_XOR: return "LexemType_XOR";

		case	LexemType_EQUAL: return "LexemType_EQUAL";
		case	LexemType_NOTEQUAL: return "LexemType_NOTEQUAL";

		case	LexemType_MORE: return "LexemType_MORE";
		case	LexemType_LESS: return "LexemType_LESS";

		case	LexemType_LESS_OR_EQUAL: return "LexemType_LESS_OR_EQUAL";
		case	LexemType_MORE_OR_EQUAL: return "LexemType_MORE_OR_EQUAL";

			//Assign
		case	LexemType_ASSIGN: return "LexemType_ASSIGN"; //("=" or "IS")

			//Symbols
		case	LexemType_POINT: return "LexemType_POINT";
		case	LexemType_COMMA: return "LexemType_COMMA";
		case	LexemType_SEMICOLON: return "LexemType_SEMICOLON";
		case	LexemType_COLON: return "LexemType_COLON";
		case	LexemTupe_QUESTION_MARK: return "LexemTupe_QUESTION_MARK";
		case	LexemType_EXCLAMATION_MARK: return "LexemType_EXCLAMATION_MARK";

		case	LexemType_LEFT_BRACKET: return "LexemType_LEFT_BRACKET";//"("
		case	LexemType_RIGHT_BRACKET: return "LexemType_RIGHT_BRACKET";//")"
		case	LexemType_LEFT_BRACE: return "LexemType_LEFT_BRACE"; //"{"
		case	LexemType_RIGHT_BRACE: return "LexemType_RIGHT_BRACE";//"}"
		case	LexemType_LEFT_SQUARE_BRACKET: return "LexemType_LEFT_SQUARE_BRACKET";//"["
		case	LexemType_RIGHT_SQUARE_BRACKET: return "LexemType_RIGHT_SQUARE_BRACKET";//"]"

		case	LexemType_APOSTROPHE: return "LexemType_APOSTROPHE";
		case	LexemType_QUOTES: return "LexemType_QUOTES"; // "
		case	LexemType_SLASH: return "LexemType_SLASH"; // "/"
		case	LexemType_BACKSLASH: return "LexemType_BACKSLASH"; // "\"
		case	LexemType_NUMBER_SIGN: return "LexemType_NUMBER_SIGN"; // "#"
		case	LexemType_SOBAKA: return "LexemType_SOBAKA"; //"@"

		case	LexemType_AMPERSAND: return "LexemType_AMPERSAND";
		case	LexemType_VERTICAL_BAR: return "LexemType_VERTICAL_BAR";

			//Program key words
		case	LexemType_PROGRAM: return "LexemType_PROGRAM";
		case	LexemType_LIBRARIES: return "LexemType_LIBRARIES";
		case	LexemType_HANDLERS: return "LexemType_HANDLERS";
		case	LexemType_RENDERERS: return "LexemType_RENDERERS";
		case	LexemType_SOURCES: return "LexemType_SOURCES";
		case	LexemType_SETS: return "LexemType_SETS";
		case	LexemType_ELEMENTS: return "LexemType_ELEMENTS";
		case	LexemType_TUPLES: return "LexemType_TUPLES";
		case	LexemType_AGGREGATES: return "LexemType_AGGREGATES";
		case	LexemType_ACTIONS: return "LexemType_ACTIONS";

			//Special key words for

			//Timeline operator
		case	LexemType_TIMELINE: return "LexemType_TIMELINE";
		case	LexemType_AS: return "LexemType_AS";
		case	LexemType_UNTIL: return "LexemType_UNTIL";
			//Sequence handler operator
		case	LexemType_SEQUENCE: return "LexemType_SEQUENCE";

			//IF/Case
		case	LexemType_IF: return "LexemType_IF";
		case	LexemType_THEN: return "LexemType_THEN";
		case	LexemType_ELSE: return "LexemType_ELSE";
		case	LexemType_CASE: return "LexemType_CASE";
		case	LexemType_OF: return "LexemType_OF";

			//SUBSTITUTE operator
		case	LexemType_SUBSTITUTE: return "LexemType_SUBSTITUTE";
		case	LexemType_FOR: return "LexemType_FOR";
		case	LexemType_WHEN: return "LexemType_WHEN";

			//Download/Render operator
		case	LexemType_DOWNLOAD: return "LexemType_DOWNLOAD";
		case	LexemType_FROM: return "LexemType_FROM";
		case	LexemType_WITH: return "LexemType_WITH";
		case	LexemType_UPLOAD: return "LexemType_UPLOAD";
		case	LexemType_TO: return "LexemType_TO";

			case	LexemType_RENDER: return "LexemType_RENDER";
			//TokenType_WITH,

			case	LexemType_COMMENT: return "LexemType_COMMENT";

			case	LexemType_NOTHING: return "LexemType_NOTHING";
	default:                             return "";
	}
}

	
