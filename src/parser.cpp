#include <functional>
#include <exception>
#include <variant>

#include "parser.h"


using tvec = std::vector<token>;
#ifndef cit
#define cit const_iterator
#endif /* cit */

#define ERROR_MSG(expected, got, line) "ERROR: expected: " + std::string((expected)) +  \
    ", got: " + std::string((got)) +                                                    \
    ", line: " + std::to_string((line)) + ".\n"

#define throw_error(p, expected) do {                       \
    const token &__t = get_it_val((p));                                             \
    auto __got = tt_to_string(__t.type) + ((get_vt(&__t, buffer) == tvt::STRING) ?  \
                "[val: " + get_str_val(&__t, buffer) + "]" :                        \
                "");                                                                \
    throw std::runtime_error(                               \
        ERROR_MSG((expected), __got, __t.line)              \
        );                                                  \
} while(0)

typedef struct {
    tvec *tseq;
	tvec::cit it;
	// std::string error;
	int level;
} parser;
#define eoi(p) ((p)->tseq->cend() == (p)->it)
#define get_it_val(p) (*((p)->it))
#define inc_it(p) (++((p)->it))

using grammar_rule = std::function<as_tree *(parser *)>;

#define take_token(p, pnode, ttype) do {    \
    as_tree *__node = expect((p), (ttype)); \
    pnode->add_child(__node);               \
} while(0)

#define try_to_take_token(p, pnode, ttype) do { \
    as_tree *__node = accept((p), (ttype));     \
    if (__node) {                               \
        pnode->add_child(__node);               \
    }                                           \
} while(0)

as_tree *take_token_of(parser *p, const std::vector<token_type> &types);

#define take_rule(p, pnode, rule, err_msg) do { \
    grammar_rule __rule = (rule);               \
    as_tree *__node = __rule((p));              \
    if (__node) {                               \
        pnode->add_child(__node);               \
    } else {                                    \
        throw_error(p, (err_msg));              \
    }                                           \
} while(0)

#define try_to_take_rule(p, pnode, rule) do {   \
    grammar_rule __rule = (rule);               \
    as_tree *__node = __rule((p));              \
    if (__node) {                               \
        pnode->add_child(__node);               \
    }                                           \
} while(0)

class parser_type_error: public std::runtime_error
{
public:
    parser_type_error(token_type _expected, token_type _got, int _line) :
        std::runtime_error(ERROR_MSG(tt_to_string(_expected), tt_to_string(_got), _line))
    {}
};

static ast_nt ttype_to_atype(token_type type);

static as_tree *accept(parser *p, token_type ttype);
static bool bool_accept(parser *p, token_type ttype);
static as_tree *expect(parser *p, token_type ttype);
static bool bool_expect(parser *p, token_type ttype);

static bool ebnf_multiple(parser *p, as_tree *parent_node, grammar_rule rule);
static as_tree *ebnf_select(parser *p, const std::vector<grammar_rule> &rules);

static void append_arg_list(parser *p, as_tree *parent_node);

static as_tree *prog(parser *p);
static as_tree *var_or_func_decl(parser *p);
static as_tree *var_decl_st(parser *p);
static as_tree *arr(parser *p);
static as_tree *expr(parser *p);
static as_tree *func_decl(parser *p);
static as_tree *param_list(parser *p);
static as_tree *st(parser *p);
static as_tree *expr_st(parser *p);
static as_tree *block_st(parser *p);
static as_tree *select_st(parser *p);
static as_tree *iter_st(parser *p);
static as_tree *match_st(parser *p);
static as_tree *timeline_st(parser *p);
static as_tree *download_st(parser *p);
static as_tree *updload_st(parser *p);
static as_tree *import_st(parser *p);
static as_tree *jump_st(parser *p);

static as_tree *case_handler(parser *p);
static as_tree *match_list(parser *p);
static as_tree *def_case(parser *p);

static as_tree *obj(parser *p);
static as_tree *obj_decl(parser *p);
static as_tree *field(parser *p);

static as_tree *func(parser *p);
static as_tree *lambda(parser *p);

static as_tree *continue_handler(parser *p);
static as_tree *break_handler(parser *p);
static as_tree *return_handler(parser *p);

static as_tree *assign(parser *p);
static as_tree *assign_ap(parser *p);
static as_tree *ternary(parser *p);
static as_tree *log_or(parser *p);
static as_tree *log_or_ap(parser *p);
static as_tree *log_and(parser *p);
static as_tree *log_and_ap(parser *p);
static as_tree *incl_or(parser *p);
static as_tree *incl_or_ap(parser *p);
static as_tree *excl_or(parser *p);
static as_tree *excl_or_ap(parser *p);
static as_tree *band(parser *p);
static as_tree *band_ap(parser *p);
static as_tree *eq(parser *p);
static as_tree *eq_ap(parser *p);
static as_tree *rel(parser *p);
static as_tree *rel_ap(parser *p);
static as_tree *shift(parser *p);
static as_tree *shift_ap(parser *p);
static as_tree *add(parser *p);
static as_tree *add_ap(parser *p);
static as_tree *mult(parser *p);
static as_tree *mult_ap(parser *p);
static as_tree *unary(parser *p);
static as_tree *postfix(parser *p);
static as_tree *postfix_ap(parser *p);
static as_tree *primary(parser *p);
static as_tree *id(parser *p);
static as_tree *number(parser *p);
static as_tree *string(parser *p);
static as_tree *boolean(parser *p);
static as_tree *var_or_call(parser *p);
static as_tree *parentheses(parser *p);
static as_tree *fn_call(parser *p);
static as_tree *arr_el(parser *p);

as_tree *take_token_of(parser *p, const std::vector<token_type> &types) {
    as_tree *node = nullptr;
    for(auto type : types) {
        node = accept(p, type);
        if (node) {
            return node;
        }
    }
    return nullptr;
}

as_tree *buid_tree(std::vector<token> &token_sequence) {
    try {
        parser p = {
            .tseq = &token_sequence,
            .it = token_sequence.cbegin(),
            .level = 0,
        };

        return prog(&p);
    }
    catch(std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return nullptr;
    }
}

void release_tree(as_tree *tree) {

}


static as_tree *prog(parser *p) {
    as_tree *prog_node = new as_tree(new ast_node(ast_nt::PROGRAM));
    ebnf_multiple(p, prog_node, var_or_func_decl);
    return prog_node;
}

static bool bool_accept(parser *p, token_type ttype) {
    if (eoi(p)) return false;
    const token &t = get_it_val(p);

    if (t.type == ttype) {
        inc_it(p);
        return true;
    }
    return false;
}

static as_tree *accept(parser *p, token_type ttype) {
	if (eoi(p)) {
        return nullptr;
    }

    const token &t = get_it_val(p);
    // auto str = ((get_vt(&t, buffer) == vt::STRING) ?
	// 		get_str_val(&t, buffer) :
	// 		"0");
    // if (str == "main") {
    //     int a = 0;
    // }
	if (t.type == ttype) {
		ast_nt atype = ttype_to_atype(ttype);
        ast_node *node = nullptr;
		// ast_node *node= ((get_vt(&t, buffer) == tvt::STRING) ?
        //     new ast_node(atype, get_str_val(&t, buffer)) :
        //     new ast_node(atype, get_int_val(&t, buffer)));
        switch(get_vt(&t, buffer)) {
            _SIMPLE_CASE(tvt::STRING, node, new ast_node(atype, get_str_val(&t, buffer)))
            _SIMPLE_CASE(tvt::INT, node, new ast_node(atype, get_int_val(&t, buffer)))
            _SIMPLE_CASE(tvt::FLOAT, node, new ast_node(atype, get_flt_val(&t, buffer)))
            default: {
                throw_error(p, "correct variant type");
            }
        }
		as_tree *tree = new as_tree(node);
        inc_it(p);

        // const token &t2 = get_it_val(p);
        // if (t2.line == 54) {
        //     int a = 5;
        // }
        return tree;
	}
	return nullptr;
}

static bool bool_expect(parser *p, token_type ttype) {
	if (bool_accept(p, ttype)) {
		return true;
	}

    const token &t = get_it_val(p);
    throw parser_type_error(ttype, t.type, t.line);
	// return false;
}

static as_tree *expect(parser *p, token_type ttype) {
	as_tree *tree = accept(p, ttype);

	if (tree != nullptr) {
		return tree;
	}

    const token &t = get_it_val(p);
    throw parser_type_error(ttype, t.type, t.line);
	// return nullptr;
}

static bool ebnf_multiple(parser *p, as_tree *parent_node, grammar_rule rule) {
    as_tree *node = nullptr;
    while(node = rule(p), node) {
        parent_node->add_child(node);
    }
    return true;
}

static as_tree *ebnf_select(parser *p, const std::vector<grammar_rule> &rules) {
    as_tree *node = nullptr;
    for (auto rule : rules) {
        node = rule(p);
        if (node) {
            return node;
        }
    }
    return nullptr;
}

static as_tree *ebnf_ap_main_rule(parser *p, grammar_rule next, grammar_rule ap) {
	as_tree *next_node = next(p);
	if (next_node) {
		as_tree *ap_node = ap(p);
		if (ap_node) {
			ap_node->add_child(next_node);
			return ap_node;
		}
		return next_node;
	}
	return nullptr;
}

static as_tree *ebnf_ap_recursive_rule(parser *p, const std::vector<token_type> &types, grammar_rule next, grammar_rule ap) {
	as_tree *op_node = take_token_of(p, types);
	if (op_node == nullptr) {
        return nullptr;
    }

	as_tree *node = nullptr;
	as_tree *next_node = next(p);
	as_tree *ap_node = ap(p);
	if (ap_node) {
		ap_node->add_child(next_node);
		node = ap_node;
	}
	else {
		node = next_node;
	}

	op_node->add_child(node);
	return op_node;
}

static as_tree *import_st(parser *p) {
    if (!bool_accept(p, token_type::IMPORT)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::IMPORT));
    try_to_take_token(p, main_node, token_type::HANDLER);
    take_token(p, main_node, token_type::ID);
    bool_expect(p, token_type::FROM);
    as_tree *file_name = take_token_of(p, {
        token_type::ID,
        token_type::FILE_NAME
    });
    if (file_name) {
		main_node->add_child(file_name);
	} else {
        throw_error(p, "file_name or id");
    }
    return main_node;
}

static as_tree *var_or_func_decl(parser *p) {
    return ebnf_select(p, {var_decl_st, func_decl});
}

static as_tree *var_decl_st(parser *p) {
    as_tree *main_node = accept(p, token_type::LET);
    if (!main_node) {
        return nullptr;
    }

    take_token(p, main_node, token_type::ID);
    if (bool_accept(p, token_type::ASSIGNMENT)) {
        take_rule(p, main_node, expr, "expr");
    }
    bool_expect(p, token_type::SEMICOLON);
    return main_node;
}

static as_tree *arr(parser *p) {
    if (!bool_accept(p, token_type::LEFT_SQUARE_BRACKET)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::ARRAY));
    append_arg_list(p, main_node);
    bool_expect(p, token_type::RIGHT_SQUARE_BRACKET);
    return main_node;
}

static void append_arg_list(parser *p, as_tree *parent_node) {
    as_tree *expr_node = expr(p);
    if (expr_node) {
        parent_node->add_child(expr_node);
        while(true) {
            if (!bool_accept(p, token_type::COMMA)) {
                break;
            }
            take_rule(p, parent_node, expr, "expr");
        }
    }
}

static as_tree *func_decl(parser *p) {
    as_tree *main_node = accept(p, token_type::FN);
    if (!main_node) {
        return nullptr;
    }
    take_token(p, main_node, token_type::ID);
    bool_expect(p, token_type::LEFT_BRACKET);
    try_to_take_rule(p, main_node, param_list);
    bool_expect(p, token_type::RIGHT_BRACKET);
    take_rule(p, main_node, block_st, "block");
    return main_node;
}

static as_tree *param_list(parser *p) {
    as_tree *id_node = accept(p, token_type::ID);
    if (!id_node) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::PARAM_LIST));
    if (id_node) {
    main_node->add_child(id_node);
        while(true) {
            if (!bool_accept(p, token_type::COMMA)) {
                break;
            }
            take_token(p, main_node, token_type::ID);
        }
    }
    return main_node;
}

static as_tree *st(parser *p) {
    return ebnf_select(p, {
        block_st,
        expr_st,
        var_decl_st,
        select_st,
        iter_st,
        match_st,
        timeline_st,
        download_st,
        updload_st,
        import_st,
        jump_st
    });
}

static as_tree *expr_st(parser *p) {
    as_tree *node = expr(p);
    if (node) {
        bool_expect(p, token_type::SEMICOLON);
    } else {
        bool_accept(p, token_type::SEMICOLON);
    }
    return node;
}

static as_tree *block_st(parser *p) {
    if (!bool_accept(p, token_type::LEFT_BRACE)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::BLOCK));
    ebnf_multiple(p, main_node, st);
    bool_expect(p, token_type::RIGHT_BRACE);
    return main_node;
}

static as_tree *select_st(parser *p) {
    if (!bool_accept(p, token_type::IF) ||
        !bool_expect(p, token_type::LEFT_BRACKET))
    {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::IF));
    take_rule(p, main_node, expr, "expr");
    bool_expect(p, token_type::RIGHT_BRACKET);
    take_rule(p, main_node, st, "st");
    if (bool_accept(p, token_type::ELSE)) {
        take_rule(p, main_node, st, "else_st");
    }
    return main_node;
}

static as_tree *iter_st(parser *p) {
    if (!bool_accept(p, token_type::WHILE) ||
        !bool_expect(p, token_type::LEFT_BRACKET))
    {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::WHILE));
    take_rule(p, main_node, expr, "expr");
    bool_expect(p, token_type::RIGHT_BRACKET);
    take_rule(p, main_node, st, "st");
    return main_node;
}

static as_tree *match_st(parser *p) {
    if (!bool_accept(p, token_type::WHILE) ||
        !bool_expect(p, token_type::LEFT_BRACKET))
    {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::MATCH));
    take_rule(p, main_node, expr, "expr");
    bool_expect(p, token_type::RIGHT_BRACKET);
    bool_expect(p, token_type::LEFT_BRACE);
    ebnf_multiple(p, main_node, case_handler);
    try_to_take_rule(p, main_node, def_case);
    return main_node;
}

static as_tree *case_handler(parser *p) {
    as_tree *main_node = new as_tree(new ast_node(ast_nt::CASE));
    take_rule(p, main_node, match_list, "match_list");
    bool_expect(p, token_type::ARROW);
    take_rule(p, main_node, st, "st");
    return main_node;
}

static as_tree *match_list(parser *p) {
    as_tree *main_node = new as_tree(new ast_node(ast_nt::MATCH_LIST));
    take_rule(p, main_node, expr, "expr");
    while(true) {
        if (!bool_accept(p, token_type::PIPE)) {
            break;
        }
        take_rule(p, main_node, expr, "expr");
    }
    return main_node;
}

static as_tree *def_case(parser *p) {
    if (!bool_accept(p, token_type::DEF_CASE) ||
        !bool_expect(p, token_type::ARROW))
    {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::DEF_CASE));
    take_rule(p, main_node, st, "st");
    return main_node;
}

static as_tree *timeline_st(parser *p) {
    if (!bool_accept(p, token_type::TIMELINE)) {
        return nullptr;
    }

    as_tree *main_node = new as_tree(new ast_node(ast_nt::TIMELINE));
    take_rule(p, main_node, obj, "obj");
    take_rule(p, main_node, func, "func");
    return main_node;
}

#define __load(p, load, proposal) do {                              \
    parser *__p = (p);                                              \
    if (!bool_accept(p, token_type::load)) {                        \
        return nullptr;                                             \
    }                                                               \
    as_tree *__main_node = new as_tree(new ast_node(ast_nt::load)); \
    take_rule(__p, __main_node, expr, "expr");                      \
    bool_expect(__p, token_type::proposal);                         \
    take_rule(__p, __main_node, expr, "expr");                      \
    bool_expect(__p, token_type::WITH);                             \
    take_rule(__p, __main_node, expr, "expr");                      \
    return __main_node;                                             \
} while(0)

static as_tree *download_st(parser *p) {
    __load(p, DOWNLOAD, FROM);
}

static as_tree *updload_st(parser *p) {
    __load(p, UPLOAD, TO);
}

static as_tree *jump_st(parser *p) {
    as_tree *node = ebnf_select(p, {
        continue_handler,
        break_handler,
        return_handler
    });
    if (node) {
        bool_expect(p, token_type::SEMICOLON);
    }
    return node;
}

static as_tree *continue_handler(parser *p) {
    if (!bool_accept(p, token_type::CONTINUE)) {
        return nullptr;
    }
    return new as_tree(new ast_node(ast_nt::CONTINUE));
}

static as_tree *break_handler(parser *p) {
    if (!bool_accept(p, token_type::BREAK)) {
        return nullptr;
    }
    return new as_tree(new ast_node(ast_nt::BREAK));
}

static as_tree *return_handler(parser *p) {
    if (!bool_accept(p, token_type::RETURN)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::RETURN));
    take_rule(p, main_node, expr, "expr");
    return main_node;
}

static as_tree *obj(parser *p) {
    as_tree *node = accept(p, token_type::ID);
    if (!node) {
        node = obj_decl(p);
    }
    return node;
}

static as_tree *obj_decl(parser *p) {
    if (!bool_accept(p, token_type::LEFT_BRACE)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::OBJ_DECL));
    as_tree *field_node = field(p);
    if (field_node) {
        main_node->add_child(field_node);
        while(true) {
            if (!bool_accept(p, token_type::COMMA)) {
                break;
            }
            take_rule(p, main_node, field, "field");
        }
    }
    bool_expect(p, token_type::RIGHT_BRACE);
    return main_node;
}

static as_tree *field(parser *p) {
    as_tree *node = expr(p);
    if (!node) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::OBJ_FIELD));
    main_node->add_child(node);
    bool_expect(p, token_type::COLON);
    take_rule(p, main_node, expr, "expr");
    return main_node;
}

static as_tree *func(parser *p) {
    as_tree *node = accept(p, token_type::ID);
    if (!node) {
        node = lambda(p);
    }
    return node;
}

static as_tree *lambda(parser *p) {
    if (!bool_accept(p, token_type::PIPE)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::LAMBDA));
    try_to_take_rule(p, main_node, param_list);
    bool_expect(p, token_type::PIPE);
    take_rule(p, main_node, block_st, "block");
    return main_node;
}

static as_tree *expr(parser *p) {
    return assign(p);
}

static as_tree *assign(parser *p) {
    return ebnf_ap_main_rule(p, ternary, assign_ap);
}

static as_tree *assign_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::ASSIGNMENT,
        token_type::DIV_ASSIGNMENT,
        token_type::PLUS_ASSIGNMENT,
        token_type::MINUS_ASSIGNMENT,
        token_type::MULT_ASSIGNMENT,
        token_type::MDIV_ASSIGNMENT,
        token_type::LEFT_SHIFT_ASSIGNMENT,
        token_type::RIGHT_SHIFT_ASSIGNMENT,
        token_type::BIN_AND_ASSIGNMENT,
        token_type::BIN_OR_ASSIGNMENT,
        token_type::BIN_NOR_ASSIGNMENT,
    }, ternary, assign_ap);
}

static as_tree *ternary(parser *p) {
	as_tree *lor_node = log_or(p);
    as_tree *qmark = accept(p, token_type::QUESTION_MARK);
    if (!qmark) {
        return lor_node;
    }

    qmark->add_child(lor_node);
    take_rule(p, qmark, expr, "expr");
    bool_expect(p, token_type::COLON);
    take_rule(p, qmark, expr, "expr");
	return qmark;
}

static as_tree *log_or(parser *p) {
    return ebnf_ap_main_rule(p, log_and, log_or_ap);
}

static as_tree *log_or_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::LOG_OR
    }, log_and, log_or_ap);
}

static as_tree *log_and(parser *p) {
    return ebnf_ap_main_rule(p, incl_or, log_and_ap);
}

static as_tree *log_and_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::LOG_AND
    }, incl_or, log_and_ap);
}

static as_tree *incl_or(parser *p) {
    return ebnf_ap_main_rule(p, excl_or, incl_or_ap);
}

static as_tree *incl_or_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::BIN_OR
    }, excl_or, incl_or_ap);
}

static as_tree *excl_or(parser *p) {
    return ebnf_ap_main_rule(p, band, excl_or_ap);
}

static as_tree *excl_or_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::BIN_NOR
    }, band, excl_or_ap);
}

static as_tree *band(parser *p) {
    return ebnf_ap_main_rule(p, eq, band_ap);
}

static as_tree *band_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::BIN_AND
    }, eq, band_ap);
}

static as_tree *eq(parser *p) {
    return ebnf_ap_main_rule(p, rel, eq_ap);
}

static as_tree *eq_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::EQUAL,
        token_type::NOT_EQUAL
    }, rel, eq_ap);
}

static as_tree *rel(parser *p) {
    return ebnf_ap_main_rule(p, shift, rel_ap);
}

static as_tree *rel_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::LESS,
        token_type::MORE,
        token_type::LESS_EQUAL,
        token_type::MORE_EQUAL,
    }, shift, rel_ap);
}

static as_tree *shift(parser *p) {
    return ebnf_ap_main_rule(p, add, shift_ap);
}

static as_tree *shift_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::LEFT_SHIFT,
        token_type::RIGHT_SHIFT,
    }, add, shift_ap);
}

static as_tree *add(parser *p) {
    return ebnf_ap_main_rule(p, mult, add_ap);
}

static as_tree *add_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::PLUS,
        token_type::MINUS,
    }, mult, add_ap);
}

static as_tree *mult(parser *p) {
    return ebnf_ap_main_rule(p, unary, mult_ap);
}

static as_tree *mult_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::MULT,
        token_type::DIV,
        token_type::MDIV,
    }, unary, mult_ap);
}

static as_tree *unary(parser *p) {
	as_tree *op_node = take_token_of(p, {
        token_type::PLUS,
        token_type::MINUS,
        token_type::NOT,
        token_type::BIN_NOT,
        token_type::INCREM,
        token_type::DECREM,
    });
	as_tree *post_node = postfix(p);

	if (op_node) {
		op_node->add_child(post_node);
		return op_node;
	}
	return post_node;
}

static as_tree *postfix(parser *p) {
    return ebnf_ap_main_rule(p, primary, postfix_ap);
}

static as_tree *postfix_ap(parser *p) {
    return ebnf_ap_recursive_rule(p, {
        token_type::INCREM,
        token_type::DECREM,
        token_type::DOT,
    }, primary, postfix_ap);
}

static as_tree *primary(parser *p) {
    return ebnf_select(p, {
        number,
        string,
        boolean,
        parentheses,
        var_or_call,
        obj_decl,
        lambda,
        arr
    });
}

static as_tree *id(parser *p) {
    return accept(p, token_type::ID);
}

static as_tree *number(parser *p) {
    return accept(p, token_type::NUMBER);
}

static as_tree *string(parser *p) {
    return accept(p, token_type::STRING);
}

static as_tree *boolean(parser *p) {
    return accept(p, token_type::LOGIC);
}

static as_tree *var_or_call(parser *p) {
    as_tree *main_node = id(p);
    if (main_node) {
        as_tree *node = fn_call(p);
        if (node) {
            main_node->add_child(node);
        } else {
            node = arr_el(p);
            if (node) {
                main_node->add_child(node);
            }
        }
    }
    return main_node;
}

static as_tree *parentheses(parser *p) {
    if (!bool_accept(p, token_type::LEFT_BRACKET)) {
        return nullptr;
    }
    as_tree *node = expr(p);
    bool_expect(p, token_type::RIGHT_BRACKET);
    return node;
}

static as_tree *fn_call(parser *p) {
    if (!bool_accept(p, token_type::LEFT_BRACKET)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::FN_CALL));
    append_arg_list(p, main_node);
    bool_expect(p, token_type::RIGHT_BRACKET);
    return main_node;
}

static as_tree *arr_el(parser *p) {
    if (!bool_accept(p, token_type::LEFT_SQUARE_BRACKET)) {
        return nullptr;
    }
    as_tree *main_node = new as_tree(new ast_node(ast_nt::ARR_EL));
    take_rule(p, main_node, expr, "expr");
    bool_expect(p, token_type::RIGHT_SQUARE_BRACKET);
    return main_node;
}

static ast_nt ttype_to_atype(token_type ttype) {
    ast_nt atype = ast_nt::NONE;
    switch(ttype) {
        _SIMPLE_CASE(token_type::HANDLER, atype, ast_nt::HANDLER)
        _SIMPLE_CASE(token_type::IMPORT, atype, ast_nt::IMPORT)
        _SIMPLE_CASE(token_type::FROM, atype, ast_nt::FROM)
        _SIMPLE_CASE(token_type::IF, atype, ast_nt::IF)
        _SIMPLE_CASE(token_type::ELSE, atype, ast_nt::ELSE)
        _SIMPLE_CASE(token_type::WHILE, atype, ast_nt::WHILE)
        _SIMPLE_CASE(token_type::MATCH, atype, ast_nt::MATCH)
        _SIMPLE_CASE(token_type::DEF_CASE, atype, ast_nt::DEF_CASE)
        _SIMPLE_CASE(token_type::TIMELINE, atype, ast_nt::TIMELINE)
        _SIMPLE_CASE(token_type::DOWNLOAD, atype, ast_nt::DOWNLOAD)
        _SIMPLE_CASE(token_type::UPLOAD, atype, ast_nt::UPLOAD)
        _SIMPLE_CASE(token_type::TO, atype, ast_nt::TO)
        _SIMPLE_CASE(token_type::FN, atype, ast_nt::FN)
        _SIMPLE_CASE(token_type::LET, atype, ast_nt::LET)
        _SIMPLE_CASE(token_type::LOGIC, atype, ast_nt::LOGIC)
        _SIMPLE_CASE(token_type::WITH, atype, ast_nt::WITH)
        _SIMPLE_CASE(token_type::CONTINUE, atype, ast_nt::CONTINUE)
        _SIMPLE_CASE(token_type::BREAK, atype, ast_nt::BREAK)
        _SIMPLE_CASE(token_type::RETURN, atype, ast_nt::RETURN)

        _SIMPLE_CASE(token_type::NOT, atype, ast_nt::NOT)
        _SIMPLE_CASE(token_type::BIN_NOT, atype, ast_nt::BIN_NOT)

        _SIMPLE_CASE(token_type::ID, atype, ast_nt::ID)
        _SIMPLE_CASE(token_type::STRING, atype, ast_nt::STRING)
        _SIMPLE_CASE(token_type::NUMBER, atype, ast_nt::NUMBER)

        _SIMPLE_CASE(token_type::SEMICOLON, atype, ast_nt::SEMICOLON)
        _SIMPLE_CASE(token_type::LEFT_BRACE, atype, ast_nt::LEFT_BRACE)
        _SIMPLE_CASE(token_type::RIGHT_BRACE, atype, ast_nt::RIGHT_BRACE)
        _SIMPLE_CASE(token_type::LEFT_SQUARE_BRACKET, atype, ast_nt::LEFT_SQUARE_BRACKET)
        _SIMPLE_CASE(token_type::RIGHT_SQUARE_BRACKET, atype, ast_nt::RIGHT_SQUARE_BRACKET)
        _SIMPLE_CASE(token_type::COMMA, atype, ast_nt::COMMA)
        _SIMPLE_CASE(token_type::DOT, atype, ast_nt::DOT)
        _SIMPLE_CASE(token_type::COLON, atype, ast_nt::COLON)
        _SIMPLE_CASE(token_type::LEFT_BRACKET, atype, ast_nt::LEFT_BRACKET)
        _SIMPLE_CASE(token_type::RIGHT_BRACKET, atype, ast_nt::RIGHT_BRACKET)
        _SIMPLE_CASE(token_type::EQUAL, atype, ast_nt::EQUAL)
        _SIMPLE_CASE(token_type::NOT_EQUAL, atype, ast_nt::NOT_EQUAL)
        _SIMPLE_CASE(token_type::LESS_EQUAL, atype, ast_nt::LESS_EQUAL)
        _SIMPLE_CASE(token_type::MORE_EQUAL, atype, ast_nt::MORE_EQUAL)

        _SIMPLE_CASE(token_type::ASSIGNMENT, atype, ast_nt::ASSIGNMENT)
        _SIMPLE_CASE(token_type::DIV_ASSIGNMENT, atype, ast_nt::DIV_ASSIGNMENT)
        _SIMPLE_CASE(token_type::PLUS_ASSIGNMENT, atype, ast_nt::PLUS_ASSIGNMENT)
        _SIMPLE_CASE(token_type::MINUS_ASSIGNMENT, atype, ast_nt::MINUS_ASSIGNMENT)
        _SIMPLE_CASE(token_type::MULT_ASSIGNMENT, atype, ast_nt::MULT_ASSIGNMENT)
        _SIMPLE_CASE(token_type::MDIV_ASSIGNMENT, atype, ast_nt::MDIV_ASSIGNMENT)
        _SIMPLE_CASE(token_type::LEFT_SHIFT_ASSIGNMENT, atype, ast_nt::LEFT_SHIFT_ASSIGNMENT)
        _SIMPLE_CASE(token_type::RIGHT_SHIFT_ASSIGNMENT, atype, ast_nt::RIGHT_SHIFT_ASSIGNMENT)
        _SIMPLE_CASE(token_type::BIN_AND_ASSIGNMENT, atype, ast_nt::BIN_AND_ASSIGNMENT)
        _SIMPLE_CASE(token_type::BIN_OR_ASSIGNMENT, atype, ast_nt::BIN_OR_ASSIGNMENT)
        _SIMPLE_CASE(token_type::BIN_NOR_ASSIGNMENT, atype, ast_nt::BIN_NOR_ASSIGNMENT)

        _SIMPLE_CASE(token_type::DIV, atype, ast_nt::DIV)
        _SIMPLE_CASE(token_type::PLUS, atype, ast_nt::PLUS)
        _SIMPLE_CASE(token_type::MINUS, atype, ast_nt::MINUS)
        _SIMPLE_CASE(token_type::MULT, atype, ast_nt::MULT)
        _SIMPLE_CASE(token_type::MDIV, atype, ast_nt::MDIV)
        _SIMPLE_CASE(token_type::LESS, atype, ast_nt::LESS)
        _SIMPLE_CASE(token_type::MORE, atype, ast_nt::MORE)
        _SIMPLE_CASE(token_type::BIN_AND, atype, ast_nt::BIN_AND)
        _SIMPLE_CASE(token_type::BIN_OR, atype, ast_nt::BIN_OR)
        _SIMPLE_CASE(token_type::BIN_NOR, atype, ast_nt::BIN_NOR)
        _SIMPLE_CASE(token_type::LOG_AND, atype, ast_nt::LOG_AND)
        _SIMPLE_CASE(token_type::LOG_OR, atype, ast_nt::LOG_OR)
        _SIMPLE_CASE(token_type::INCREM, atype, ast_nt::INCREM)
        _SIMPLE_CASE(token_type::DECREM, atype, ast_nt::DECREM)

        _SIMPLE_CASE(token_type::ARROW, atype, ast_nt::ARROW)
        _SIMPLE_CASE(token_type::LEFT_SHIFT_OPERATOR, atype, ast_nt::LEFT_SHIFT_OPERATOR)
        _SIMPLE_CASE(token_type::RIGHT_SHIFT_OPERATOR, atype, ast_nt::RIGHT_SHIFT_OPERATOR)

        _SIMPLE_CASE(token_type::LEFT_SHIFT, atype, ast_nt::LEFT_SHIFT)
        _SIMPLE_CASE(token_type::RIGHT_SHIFT, atype, ast_nt::RIGHT_SHIFT)
        _SIMPLE_CASE(token_type::QUESTION_MARK, atype, ast_nt::QUESTION_MARK)
        _SIMPLE_CASE(token_type::FILE_NAME, atype, ast_nt::FILE_NAME)

        _SIMPLE_CASE(token_type::NONE, atype, ast_nt::NONE)
    }
    return atype;
}

std::string at_to_string(ast_nt type) {
#define C2S(x) case ast_nt::x: return #x;
    switch(type) {
        C2S(PROGRAM)

        C2S(PARAM_LIST)
        C2S(ARG_LIST)

        C2S(FN_CALL)
        C2S(ARRAY)
        C2S(ARR_EL)

        C2S(BLOCK)
        C2S(IF)
        C2S(WHILE)
        C2S(MATCH)
        C2S(CASE)
        C2S(DEF_CASE)
        C2S(MATCH_LIST)

        C2S(OBJ_DECL)
        C2S(OBJ_FIELD)

        C2S(LAMBDA)

        C2S(HANDLER)
        C2S(IMPORT)
        C2S(FROM)
        C2S(ELSE)
        C2S(TIMELINE)
        C2S(DOWNLOAD)
        C2S(UPLOAD)
        C2S(TO)
        C2S(FN)
        C2S(LET)
        C2S(LOGIC)
        C2S(WITH)
        C2S(CONTINUE)
        C2S(BREAK)
        C2S(RETURN)

        C2S(NOT)
        C2S(BIN_NOT)

        C2S(ID)
        C2S(STRING)
        C2S(NUMBER)

        C2S(SEMICOLON)
        C2S(LEFT_BRACE)
        C2S(RIGHT_BRACE)
        C2S(LEFT_SQUARE_BRACKET)
        C2S(RIGHT_SQUARE_BRACKET)
        C2S(COMMA)
        C2S(DOT)
        C2S(COLON)
        C2S(LEFT_BRACKET)
        C2S(RIGHT_BRACKET)
        C2S(EQUAL)
        C2S(NOT_EQUAL)
        C2S(LESS_EQUAL)
        C2S(MORE_EQUAL)

        C2S(ASSIGNMENT)
        C2S(DIV_ASSIGNMENT)
        C2S(PLUS_ASSIGNMENT)
        C2S(MINUS_ASSIGNMENT)
        C2S(MULT_ASSIGNMENT)
        C2S(MDIV_ASSIGNMENT)
        C2S(LEFT_SHIFT_ASSIGNMENT)
        C2S(RIGHT_SHIFT_ASSIGNMENT)
        C2S(BIN_AND_ASSIGNMENT)
        C2S(BIN_OR_ASSIGNMENT)
        C2S(BIN_NOR_ASSIGNMENT)

        C2S(DIV)
        C2S(PLUS)
        C2S(MINUS)
        C2S(MULT)
        C2S(MDIV)
        C2S(LESS)
        C2S(MORE)
        C2S(BIN_AND)
        C2S(BIN_OR)
        C2S(BIN_NOR)
        C2S(LOG_AND)
        C2S(LOG_OR)
        C2S(INCREM)
        C2S(DECREM)

        C2S(ARROW)
        C2S(LEFT_SHIFT_OPERATOR)
        C2S(RIGHT_SHIFT_OPERATOR)

        C2S(LEFT_SHIFT)
        C2S(RIGHT_SHIFT)
        C2S(QUESTION_MARK)
        C2S(FILE_NAME)

        C2S(NONE)
    }
#undef C2S
    return "";
}