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
    // END_OPERATOR,

    LEFT_SHIFT,
    RIGHT_SHIFT,
};

static std::map<std::string, token_type> key_words = {
    {"handler",  token_type::KW_HANDLER},
    {"library",  token_type::KW_LIBRARY},
    {"from",     token_type::KW_FROM},
    {"if",       token_type::KW_IF},
    {"while",    token_type::KW_WHILE},
    {"match",    token_type::KW_MATCH},
    {"timeline", token_type::KW_TIMELINE},
    {"download", token_type::KW_DOWNLOAD},
    {"updload",  token_type::KW_UPLOAD},
    {"to",       token_type::KW_TO},
    {"fn",       token_type::KW_FN},
    {"let",      token_type::KW_LET},
};

static std::vector<token_type> delay_types = {
    token_type::KW_HANDLER,
    token_type::KW_LIBRARY,
    token_type::KW_FROM,
    token_type::KW_IF,
    token_type::KW_WHILE,
    token_type::KW_MATCH,
    token_type::KW_TIMELINE,
    token_type::KW_DOWNLOAD,
    token_type::KW_UPLOAD,
    token_type::KW_TO,
    token_type::KW_FN,
    token_type::KW_LET,

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
};

static std::vector<lexer_state> wrt_states = {
    lexer_state::WORD,
	lexer_state::STRING,
	lexer_state::NUM_OR_WORD,
};

using str_cit = std::string::const_iterator;

static int line = 0;
static lexer_state state = lexer_state::NONE;

static inline char get_next_char(str_cit &it) {
    return *(++it);
}

static void get_to_next_line(str_cit &it) {
    ++line;
    while(get_next_char(it) != '\n') {}
    get_next_char(it);
}

static void get_to_next_nospace(str_cit &it) {
    char c = 0;
    while(c = get_next_char(it), isspace(c) || c == '\n')
        if (c == '\n')
            ++line;
}

static void get_to_next(str_cit &it) {
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

static void file_promotion(str_cit &it) {
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
    auto kw_it = key_words.find(buf);
    if (kw_it != key_words.end()) {
        ttype = kw_it->second;
    } else if (!is_name_part(c)) {
        ttype = token_type::ID;
    }
    return ttype;
}

static token_type state_num_or_word_handle(const std::string &buf, char c) {
    token_type ttype = token_type::NONE;
    if (!isdigit(c)) {
        if (is_name_part(c)) {
            state = lexer_state::WORD;
            ttype = state_word_handle(buf, c);
        } else {
            ttype = token_type::NUMBER;
        }
    }
    return ttype;
}

static token_type state_div_operator_handler(str_cit &it) {
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
        _SIMPLE_CASE('=', ttype, token_type::LEFT_SHIFT)
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
        _SIMPLE_CASE('>', state, lexer_state::RIGHT_SHIFT)
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
        default:
            ttype = token_type::BIN_AND;
    }
    return ttype;
}

static token_type state_bin_or_operator_handler(char c) {
    token_type ttype = token_type::NONE;
    switch(c) {
        _SIMPLE_CASE('=', ttype, token_type::BIN_OR_ASSIGNMENT)
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



int split_tokens(std::fstream &file, std::vector<token> &token_sequence) {
    std::string file_contents = std::string(std::istreambuf_iterator<char>(file),
                                            std::istreambuf_iterator<char>());
    str_cit it = file_contents.cbegin();
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

            _SIMPLE_CASE(lexer_state::LEFT_SHIFT, ttype, state_left_shift_handler(*it))
            _SIMPLE_CASE(lexer_state::RIGHT_SHIFT, ttype, state_right_shift_handler(*it))
        }

        if (ttype != token_type::NONE) {
            token_sequence.emplace_back((store_buf(ttype)) ? buf : "", ttype, line);
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
    return 0;
}


// std::string to_string(token_type type) {
//     return
// }