#pragma once
#include <string>
#include <vector>
#include <fstream>

#define IS_NAME_START(c)  (isalpha( (c) ) || (c) == '_')

#define IS_NAME( c )	  ( IS_NAME_START( c ) || isdigit(c) )

#define IS_STRING_LITERAL_START( c )	  ( (c) == '\'' || (c) == '\"' )
	

typedef enum {
	//Base_Symbols
	LexemType_INTEGER,
	LexemType_REAL,

	LexemType_NUMBER,

	LexemType_STRING_LITERAL,
	LexemType_NAME,

	//True/False
	LexemType_LOGIC, //BOOLEAN

	//Math_Operators
	LexemType_PLUS,
	LexemType_MINUS,
	LexemType_MULT,
	LexemType_DIV,

	LexemType_MOD, //May be

	LexemType_CARET,

	//Logical_Operators
	LexemType_AND,
	LexemType_OR,
	LexemType_NOT,
	LexemType_XOR,

	LexemType_EQUAL,
	LexemType_NOTEQUAL,

	LexemType_MORE,
	LexemType_LESS,

	LexemType_LESS_OR_EQUAL,
	LexemType_MORE_OR_EQUAL,

	//Assign
	LexemType_ASSIGN, //("=" or "IS")

	//Symbols
	LexemType_POINT,
	LexemType_COMMA,
	LexemType_SEMICOLON,
	LexemType_COLON,
	LexemTupe_QUESTION_MARK,
	LexemType_EXCLAMATION_MARK,

	LexemType_LEFT_BRACKET,//"("
	LexemType_RIGHT_BRACKET,//")"
	LexemType_LEFT_BRACE, //"{"
	LexemType_RIGHT_BRACE,//"}"
	LexemType_LEFT_SQUARE_BRACKET,//"["
	LexemType_RIGHT_SQUARE_BRACKET,//"]"

	LexemType_APOSTROPHE,
	LexemType_QUOTES, // "
	LexemType_SLASH, // "/"
	LexemType_BACKSLASH, // "\"
	LexemType_NUMBER_SIGN, // "#"
	LexemType_SOBAKA, //"@"

	LexemType_AMPERSAND,
	LexemType_VERTICAL_BAR,

	//Program key words
	LexemType_PROGRAM,
	LexemType_LIBRARIES, 
	LexemType_HANDLERS,
	LexemType_RENDERERS,
	LexemType_SOURCES,
	LexemType_SETS,
	LexemType_ELEMENTS,
	LexemType_TUPLES,
	LexemType_AGGREGATES,
	LexemType_ACTIONS,

	//Special key words for

	//Timeline operator
	LexemType_TIMELINE,
	LexemType_AS,
	LexemType_UNTIL,
	//Sequence handler operator
	LexemType_SEQUENCE,

	//IF/Case/while
	LexemType_IF,
	LexemType_THEN,
	LexemType_ELSE,
	LexemType_SWITCH,
	LexemType_DEFAULT,
	LexemType_CASE,
	LexemType_OF,
	LexemType_WHILE,

	//SUBSTITUTE operator
	LexemType_SUBSTITUTE,
	LexemType_FOR,
	LexemType_WHEN,

	//Download/Render operator
	LexemType_DOWNLOAD,
	LexemType_FROM,
	LexemType_WITH,
	LexemType_UPLOAD,
	LexemType_TO,

	LexemType_RENDER,
	LexemType_PRINT,
	//TokenType_WITH,

	LexemType_COMMENT,

	LexemType_NOTHING,
} LexemType;


class Lexem
{
	std::string	buffer;
	LexemType type;
	int line;

public:

	// конструктор по умолчанию
	Lexem() {
		type = LexemType_NOTHING;
	}
	// конструктор с заданием параметров
	Lexem(std::string b, LexemType t, int positionInFile) : buffer(b), type(t), line(positionInFile) {
	}


	std::string GetBuffer() const {
		return buffer;
	}

	LexemType GetType() const {
		return type;
	}

	int GetLine() {
		return line;
	}

	void SetBuffer(std::string b) {
		buffer = b;
	}

	void SetType(LexemType t) {
		type = t;
	}

	void SetLine(int l) {
		line = l;
	}

};



//Main function
int Lexer_splitTokens(std::fstream *fs, std::vector<Lexem>* lexem_sequence);

//just skip spaces
inline static int IgnoreTabNewlinesAndSpaces(std::fstream *fs);


//Выделить оператор
inline static int LexemOperator(std::fstream *fs, Lexem *lexem_to_add);

//Выделить имя
inline static int LexemName(std::fstream *fs, Lexem *lexem_to_add);

//Выделить цифру
inline static int LexemDigit(std::fstream *fs, Lexem *lexem_to_add);

//Print all
 void LexemPrint(std::vector<Lexem>* lexem_sequence);

 std::string LexemType_toString(LexemType type);

