#include "lexer.h"

#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#define is_name_part(c) \
    (isalpha((c)) || c == '_' || c == '-' || isdigit((c)))

#define contains(container, val) \
    (std::find((container).begin(), (container).end(), (val)) != (container).end())

enum class lexer_state {
	NONE,
	WORD,
    NUM_OR_WORD,
	STRING,
	// COMMENT_OR_ASSIGN,

    // START_OPERATOR,
    DIV_OPERATOR,
    PLUS_OPERATOR,
    MINUS_OPERATOR,
    MULT_OPERATOR,
    MDIV_OPERATOR,
    LESS_OPERATOR,
    MORE_OPERATOR,
    BIN_AND_OPERATOR,
    BIN_OR_OPERATOR,
    BIN_NOR_OPERATOR,
    NOT_OPERATOR,
    // END_OPERATOR,

    LEFT_SHIFT,
    RIGHT_SHIFT,
};

static std::map<std::string, token_type> key_words = {
    {"handler",  token_type::HANDLER},
    {"library",  token_type::LIBRARY},
    {"from",     token_type::FROM},
    {"if",       token_type::IF},
    {"else",     token_type::ELSE},
    {"while",    token_type::WHILE},
    {"match",    token_type::MATCH},
    {"def",      token_type::DEF_CASE},
    {"timeline", token_type::TIMELINE},
    {"download", token_type::DOWNLOAD},
    {"updload",  token_type::UPLOAD},
    {"to",       token_type::TO},
    {"fn",       token_type::FN},
    {"let",      token_type::LET},
    {"true",     token_type::TRUE},
    {"false",    token_type::FALSE},
    {"with",     token_type::WITH},
    {"continue", token_type::CONTINUE},
    {"break",    token_type::BREAK},
    {"return",   token_type::RETURN},
};

static std::vector<token_type> delay_types = {
    token_type::HANDLER,
    token_type::LIBRARY,
    token_type::FROM,
    token_type::IF,
    token_type::WHILE,
    token_type::MATCH,
    token_type::TIMELINE,
    token_type::DOWNLOAD,
    token_type::UPLOAD,
    token_type::TO,
    token_type::FN,
    token_type::LET,
    token_type::ID,
    token_type::NUMBER,
    token_type::DIV,
    token_type::PLUS,
    token_type::MINUS,
    token_type::MULT,
    token_type::MDIV,
    token_type::LESS,
    token_type::MORE,
    token_type::BIN_AND,
    token_type::BIN_OR,
    token_type::BIN_NOR,
    token_type::INCREM,
    token_type::DECREM,
    token_type::LEFT_SHIFT,
    token_type::RIGHT_SHIFT,
    token_type::ELSE,
    token_type::DEF_CASE,
    token_type::TRUE,
    token_type::FALSE,
    token_type::WITH,
    token_type::CONTINUE,
    token_type::BREAK,
    token_type::RETURN,
};

static std::vector<lexer_state> wrt_states = {
    lexer_state::WORD,
	lexer_state::STRING,
	lexer_state::NUM_OR_WORD,
};

#define cit const_iterator

static int line = 1;
static lexer_state state = lexer_state::NONE;

static bool point_in_num = false;

static inline char get_next_char(std::string::cit &it) {
    return *(++it);
}

static void get_to_next_line(std::string::cit &it) {
    ++line;
    while(get_next_char(it) != '\n') {}
    get_next_char(it);
}

static void get_to_next_nospace(std::string::cit &it) {
    char c = 0;
    while(c = get_next_char(it), isspace(c) || c == '\n')
        if (c == '\n')
            ++line;
}

static void get_to_next(std::string::cit &it) {
    ++it;
}

static void write_to_buf(std::string &buf, char c) {
    switch(state) {
        case lexer_state::STRING:
            if (!(buf.length() == 1 && buf.back() == '\"'))
                buf.push_back(c);
            break;
        default:
            if (c != ' ')
                buf.push_back(c);
    }
}

static bool store_buf(token_type ttype) {
    switch(ttype) {
        case token_type::ID:
            return true;
        default:
            return false;
    }
}

static void file_promotion(std::string::cit &it) {
    switch(state) {
        case lexer_state::STRING:
        case lexer_state::WORD:
        case lexer_state::NUM_OR_WORD:
            get_to_next(it);
            break;
        default:
            get_to_next_nospace(it);
    }
}

#define _SIMPLE_CASE(match_val, var, val)       \
    case match_val:                             \
        var = val;                              \
        break;                                  \

static token_type state_none_handle(const std::string &buf, char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE(';', ttype, token_type::SEMICOLON)
        _SIMPLE_CASE('{', ttype, token_type::LEFT_BRACE)
        _SIMPLE_CASE('}', ttype, token_type::RIGHT_BRACE)
        _SIMPLE_CASE('[', ttype, token_type::LEFT_SQUARE_BRACKET)
        _SIMPLE_CASE(']', ttype, token_type::RIGHT_SQUARE_BRACKET)
        _SIMPLE_CASE(',', ttype, token_type::COMMA)
        _SIMPLE_CASE('.', ttype, token_type::DOT)
        _SIMPLE_CASE(':', ttype, token_type::COLON)
        _SIMPLE_CASE('(', ttype, token_type::LEFT_BRACKET)
        _SIMPLE_CASE(')', ttype, token_type::RIGHT_BRACKET)
        _SIMPLE_CASE('=', ttype, token_type::EQUAL)
        _SIMPLE_CASE('~', ttype, token_type::BIN_NOT)
        _SIMPLE_CASE('?', ttype, token_type::QUESTION_MARK)
        _SIMPLE_CASE('\"', state, lexer_state::STRING)
        _SIMPLE_CASE('/', state, lexer_state::DIV_OPERATOR)
        _SIMPLE_CASE('+', state, lexer_state::PLUS_OPERATOR)
        _SIMPLE_CASE('-', state, lexer_state::MINUS_OPERATOR)
        _SIMPLE_CASE('*', state, lexer_state::MULT_OPERATOR)
        _SIMPLE_CASE('%', state, lexer_state::MDIV_OPERATOR)
        _SIMPLE_CASE('<', state, lexer_state::LESS_OPERATOR)
        _SIMPLE_CASE('>', state, lexer_state::MORE_OPERATOR)
        _SIMPLE_CASE('&', state, lexer_state::BIN_AND_OPERATOR)
        _SIMPLE_CASE('|', state, lexer_state::BIN_OR_OPERATOR)
        _SIMPLE_CASE('^', state, lexer_state::BIN_NOR_OPERATOR)
        _SIMPLE_CASE('!', state, lexer_state::NOT_OPERATOR)
        default: {
            if (isdigit(c)) {
                state = lexer_state::NUM_OR_WORD;
            } else if (is_name_part(c)) {
                state = lexer_state::WORD;
            }
        }
    }
    return ttype;
}

static token_type state_string_handle(const std::string &buf, char c) {
    token_type ttype = token_type::NONE;
    if ((buf.length() > 0 && c == '\"' && buf.back() != '\\') ||
        (buf.length() == 0 && c == '\"')) {
        ttype = token_type::STRING;
    }

    return ttype;
}

static token_type state_word_handle(const std::string &buf, char c) {
    token_type ttype = token_type::NONE;
    auto it = key_words.find(buf);
    if (it != key_words.end()) {
        ttype = it->second;
    } else if (!is_name_part(c)) {
        ttype = token_type::ID;
    }
    return ttype;
}

static token_type state_num_or_word_handle(const std::string &buf, char c) {
    token_type ttype = token_type::NONE;
    if (!isdigit(c)) {
        if (!point_in_num && c == '.') {
            point_in_num = true;
        } else if (is_name_part(c)) {
            state = lexer_state::WORD;
            ttype = state_word_handle(buf, c);
        } else {
            ttype = token_type::NUMBER;
        }
    }
    return ttype;
}

static token_type state_div_operator_handler(std::string::cit &it) {
    token_type ttype = token_type::NONE;
    switch(*it) {
        case '/':
            state = lexer_state::NONE;
            get_to_next_line(it);
            break;
        _SIMPLE_CASE('=', ttype, token_type::DIV_ASSIGNMENT)
        default:
            ttype = token_type::DIV;
    }
    return ttype;
}

static token_type state_plus_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::PLUS_ASSIGNMENT)
        _SIMPLE_CASE('+', ttype, token_type::INCREM)
        default:
            ttype = token_type::PLUS;
    }
    return ttype;
}

static token_type state_minus_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::MINUS_ASSIGNMENT)
        _SIMPLE_CASE('-', ttype, token_type::DECREM)
        _SIMPLE_CASE('>', ttype, token_type::ARROW)
        default:
            ttype = token_type::MINUS;
    }
    return ttype;
}

static token_type state_mult_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::MULT_ASSIGNMENT)
        default:
            ttype = token_type::MULT;
    }
    return ttype;
}

static token_type state_mdiv_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::MDIV_ASSIGNMENT)
        default:
            ttype = token_type::MDIV;
    }
    return ttype;
}

static token_type state_less_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::LESS_EQUAL)
        default:
            ttype = token_type::LESS;
    }
    return ttype;
}

static token_type state_left_shift_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::LEFT_SHIFT_ASSIGNMENT)
        default:
            ttype = token_type::LEFT_SHIFT;
    }
    return ttype;
}

static token_type state_more_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::MORE_EQUAL)
        default:
            ttype = token_type::MORE;
    }
    return ttype;
}

static token_type state_right_shift_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::RIGHT_SHIFT_ASSIGNMENT)
        default:
            ttype = token_type::RIGHT_SHIFT;
    }
    return ttype;
}

static token_type state_bin_and_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::BIN_AND_ASSIGNMENT)
        _SIMPLE_CASE('&', ttype, token_type::LOG_AND)
        default:
            ttype = token_type::BIN_AND;
    }
    return ttype;
}

static token_type state_bin_or_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::BIN_OR_ASSIGNMENT)
        _SIMPLE_CASE('|', ttype, token_type::LOG_OR)
        default:
            ttype = token_type::BIN_OR;
    }
    return ttype;
}

static token_type state_bin_nor_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::BIN_NOR_ASSIGNMENT)
        default:
            ttype = token_type::BIN_NOR;
    }
    return ttype;
}

static token_type state_not_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::NOT_EQUAL)
        default:
            ttype = token_type::NOT;
    }
    return ttype;
}

int split_tokens(std::fstream &file, std::vector<token> &token_sequence) {
    std::string file_contents = std::string(std::istreambuf_iterator<char>(file),
                                            std::istreambuf_iterator<char>());
    std::string::cit it = file_contents.cbegin();
    std::string buf = "";
    token_type ttype = token_type::NONE;
    bool get_next = true;

    while(it != file_contents.end()) {
        get_next = true;
        switch(state) {
            _SIMPLE_CASE(lexer_state::NONE, ttype, state_none_handle(buf, *it))
            _SIMPLE_CASE(lexer_state::STRING, ttype, state_string_handle(buf, *it))
            _SIMPLE_CASE(lexer_state::WORD, ttype, state_word_handle(buf, *it))
            _SIMPLE_CASE(lexer_state::NUM_OR_WORD, ttype, state_num_or_word_handle(buf, *it))

            _SIMPLE_CASE(lexer_state::DIV_OPERATOR, ttype, state_div_operator_handler(it))
            _SIMPLE_CASE(lexer_state::PLUS_OPERATOR, ttype, state_plus_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::MINUS_OPERATOR, ttype, state_minus_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::MULT_OPERATOR, ttype, state_mult_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::MDIV_OPERATOR, ttype, state_mdiv_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::LESS_OPERATOR, ttype, state_less_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::MORE_OPERATOR, ttype, state_more_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::BIN_AND_OPERATOR, ttype, state_bin_and_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::BIN_OR_OPERATOR, ttype, state_bin_or_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::BIN_NOR_OPERATOR, ttype, state_bin_nor_operator_handler(*it))
            _SIMPLE_CASE(lexer_state::NOT_OPERATOR, ttype, state_not_operator_handler(*it))

            _SIMPLE_CASE(lexer_state::LEFT_SHIFT, ttype, state_left_shift_handler(*it))
            _SIMPLE_CASE(lexer_state::RIGHT_SHIFT, ttype, state_right_shift_handler(*it))
        }

        if (ttype != token_type::NONE) {
            if (store_buf(ttype)) {
                token_sequence.emplace_back(buf, ttype, line);
            } else {
                token_sequence.emplace_back((int)0, ttype, line);
            }
            if (ttype == token_type::NUMBER) {
                point_in_num = false;
            }
            // token_sequence.emplace_back((store_buf(ttype)) ? buf : (int)0, ttype, line);
            state = lexer_state::NONE;
            buf.clear();
            if (contains(delay_types, ttype)) {
                get_next = false;
            }
            ttype = token_type::NONE;
        }

        if (contains(wrt_states, state)) {
            write_to_buf(buf, *it);
        }

        if (get_next) {
            file_promotion(it);
        }
    }
    return (state == lexer_state::NONE) ? 0 : -1;
}


std::string tt_to_string(token_type type) {
    std::string buf = "";
    switch(type) {
        _SIMPLE_CASE(token_type::HANDLER, buf, "HANDLER")
        _SIMPLE_CASE(token_type::LIBRARY, buf, "LIBRARY")
        _SIMPLE_CASE(token_type::FROM, buf, "FROM")
        _SIMPLE_CASE(token_type::IF, buf, "IF")
        _SIMPLE_CASE(token_type::ELSE, buf, "ELSE")
        _SIMPLE_CASE(token_type::WHILE, buf, "WHILE")
        _SIMPLE_CASE(token_type::MATCH, buf, "MATCH")
        _SIMPLE_CASE(token_type::DEF_CASE, buf, "DEF_CASE")
        _SIMPLE_CASE(token_type::TIMELINE, buf, "TIMELINE")
        _SIMPLE_CASE(token_type::DOWNLOAD, buf, "DOWNLOAD")
        _SIMPLE_CASE(token_type::UPLOAD, buf, "UPLOAD")
        _SIMPLE_CASE(token_type::TO, buf, "TO")
        _SIMPLE_CASE(token_type::FN, buf, "FN")
        _SIMPLE_CASE(token_type::LET, buf, "LET")
        _SIMPLE_CASE(token_type::TRUE, buf, "TRUE")
        _SIMPLE_CASE(token_type::FALSE, buf, "FALSE")
        _SIMPLE_CASE(token_type::WITH, buf, "WITH")
        _SIMPLE_CASE(token_type::CONTINUE, buf, "CONTINUE")
        _SIMPLE_CASE(token_type::BREAK, buf, "BREAK")
        _SIMPLE_CASE(token_type::RETURN, buf, "RETURN")

        _SIMPLE_CASE(token_type::NOT, buf, "NOT")
        _SIMPLE_CASE(token_type::BIN_NOT, buf, "BIN_NOT")

        _SIMPLE_CASE(token_type::ID, buf, "ID")
        _SIMPLE_CASE(token_type::STRING, buf, "STRING")
        _SIMPLE_CASE(token_type::NUMBER, buf, "NUMBER")

        _SIMPLE_CASE(token_type::SEMICOLON, buf, "SEMICOLON")
        _SIMPLE_CASE(token_type::LEFT_BRACE, buf, "LEFT_BRACE")
        _SIMPLE_CASE(token_type::RIGHT_BRACE, buf, "RIGHT_BRACE")
        _SIMPLE_CASE(token_type::LEFT_SQUARE_BRACKET, buf, "LEFT_SQUARE_BRACKET")
        _SIMPLE_CASE(token_type::RIGHT_SQUARE_BRACKET, buf, "RIGHT_SQUARE_BRACKET")
        _SIMPLE_CASE(token_type::COMMA, buf, "COMMA")
        _SIMPLE_CASE(token_type::DOT, buf, "DOT")
        _SIMPLE_CASE(token_type::COLON, buf, "COLON")
        _SIMPLE_CASE(token_type::LEFT_BRACKET, buf, "LEFT_BRACKET")
        _SIMPLE_CASE(token_type::RIGHT_BRACKET, buf, "RIGHT_BRACKET")
        _SIMPLE_CASE(token_type::EQUAL, buf, "EQUAL")
        _SIMPLE_CASE(token_type::NOT_EQUAL, buf, "NOT_EQUAL")
        _SIMPLE_CASE(token_type::LESS_EQUAL, buf, "LESS_EQUAL")
        _SIMPLE_CASE(token_type::MORE_EQUAL, buf, "MORE_EQUAL")

        _SIMPLE_CASE(token_type::DIV_ASSIGNMENT, buf, "DIV_ASSIGNMENT")
        _SIMPLE_CASE(token_type::PLUS_ASSIGNMENT, buf, "PLUS_ASSIGNMENT")
        _SIMPLE_CASE(token_type::MINUS_ASSIGNMENT, buf, "MINUS_ASSIGNMENT")
        _SIMPLE_CASE(token_type::MULT_ASSIGNMENT, buf, "MULT_ASSIGNMENT")
        _SIMPLE_CASE(token_type::MDIV_ASSIGNMENT, buf, "MDIV_ASSIGNMENT")
        _SIMPLE_CASE(token_type::LEFT_SHIFT_ASSIGNMENT, buf, "LEFT_SHIFT_ASSIGNMENT")
        _SIMPLE_CASE(token_type::RIGHT_SHIFT_ASSIGNMENT, buf, "RIGHT_SHIFT_ASSIGNMENT")
        _SIMPLE_CASE(token_type::BIN_AND_ASSIGNMENT, buf, "BIN_AND_ASSIGNMENT")
        _SIMPLE_CASE(token_type::BIN_OR_ASSIGNMENT, buf, "BIN_OR_ASSIGNMENT")
        _SIMPLE_CASE(token_type::BIN_NOR_ASSIGNMENT, buf, "BIN_NOR_ASSIGNMENT")

        _SIMPLE_CASE(token_type::DIV, buf, "DIV")
        _SIMPLE_CASE(token_type::PLUS, buf, "PLUS")
        _SIMPLE_CASE(token_type::MINUS, buf, "MINUS")
        _SIMPLE_CASE(token_type::MULT, buf, "MULT")
        _SIMPLE_CASE(token_type::MDIV, buf, "MDIV")
        _SIMPLE_CASE(token_type::LESS, buf, "LESS")
        _SIMPLE_CASE(token_type::MORE, buf, "MORE")
        _SIMPLE_CASE(token_type::BIN_AND, buf, "BIN_AND")
        _SIMPLE_CASE(token_type::BIN_OR, buf, "BIN_OR")
        _SIMPLE_CASE(token_type::BIN_NOR, buf, "BIN_NOR")
        _SIMPLE_CASE(token_type::LOG_AND, buf, "LOG_AND")
        _SIMPLE_CASE(token_type::LOG_OR, buf, "LOG_OR")
        _SIMPLE_CASE(token_type::INCREM, buf, "INCREM")
        _SIMPLE_CASE(token_type::DECREM, buf, "DECREM")

        // _SIMPLE_CASE(token_type::BIN_OR, buf, "BIN_OR")

        _SIMPLE_CASE(token_type::ARROW, buf, "ARROW")
        _SIMPLE_CASE(token_type::LEFT_SHIFT_OPERATOR, buf, "LEFT_SHIFT_OPERATOR")
        _SIMPLE_CASE(token_type::RIGHT_SHIFT_OPERATOR, buf, "RIGHT_SHIFT_OPERATOR")

        _SIMPLE_CASE(token_type::LEFT_SHIFT, buf, "LEFT_SHIFT")
        _SIMPLE_CASE(token_type::RIGHT_SHIFT, buf, "RIGHT_SHIFT")
        _SIMPLE_CASE(token_type::QUESTION_MARK, buf, "QUESTION_MARK")

        _SIMPLE_CASE(token_type::NONE, buf, "NONE")
    }
    return buf;
}