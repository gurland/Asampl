#ifndef _LEXER_H
#define _LEXER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <variant>

enum class token_type {
    HANDLER,
    IMPORT,
    FROM,
    IF,
    ELSE,
    WHILE,
    MATCH,
    DEF_CASE,
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
    PIPE = BIN_OR,
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
    ASSIGNMENT,
    FILE_NAME,

    NONE,
};

#define _SIMPLE_CASE(match_val, var, val)       \
    case match_val:                             \
        var = val;                              \
        break;                                  \


#include "vt.h"
using tvt = vt;

class token {
public:
    token() : type(token_type::NONE) {}
    token(const std::string &_buffer, token_type _type, int _line) :
        buffer(_buffer), type(_type), line(_line) {}

    token(long _val, token_type _type, int _line) :
        buffer(_val), type(_type), line(_line) {}

    token(double _val, token_type _type, int _line) :
        buffer(_val), type(_type), line(_line) {}

    std::variant<long, std::string, double> buffer;
    token_type type;
    int line;
};

int split_tokens(std::fstream &fs, std::vector<token> &token_sequence);
std::string tt_to_string(token_type type);


#endif /* _LEXER_H */