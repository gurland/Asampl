#ifndef _LEXER_H
#define _LEXER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <variant>

enum class token_type {
    HANDLER,
    LIBRARY,
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
    TRUE,
    FALSE,
    WITH,
    CONTINUE,
    BREAK,
    RETURN,

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
    PIPE,
    EQUAL,

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
    BIN_NOR,
    INCREM,
    DECREM,

    ARROW,
    LEFT_SHIFT_OPERATOR,
    RIGHT_SHIFT_OPERATOR,

    LEFT_SHIFT,
    RIGHT_SHIFT,

    NONE,
};

enum class token_variant_type {
    INT = 0,
    STRING,
};

using tvt = token_variant_type;
#define get_tvt(t) ((tvt)((t)->buffer.index()))
#define get_tstr_val(t) (std::get<std::string>((t)->buffer))
// #define get_tint_val(token) (std::get<int>((token)->buffer))

class token {
public:
    token() : type(token_type::NONE) {}
    token(const std::string &_buffer, token_type _type, int _line) :
        buffer(_buffer), type(_type), line(_line) {}

    token(int, token_type _type, int _line) :
        buffer(0), type(_type), line(_line) {}

    std::variant<int, std::string> buffer;
    token_type type;
    int line;
};

int split_tokens(std::fstream &fs, std::vector<token> &token_sequence);
std::string tt_to_string(token_type type);


#endif /* _LEXER_H */