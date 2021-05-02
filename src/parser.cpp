#include <functional>
#include <exception>
#include <variant>

#include "parser.h"


using tvec = std::vector<token>;
#ifndef cit
#define cit const_iterator
#endif /* cit */

#define pick_up(p, pnode, ttype) do {       \
    as_tree *__node = expect((p), (ttype)); \
    pnode->add_child(__node);               \
} while(0)

#define ERROR_MSG(expected, got) "ERROR: expected: " + std::string((expected)) + ", got: " + std::string((got))
#define throw_error(p, expected) do {                       \
    const token &__t = get_it_val((p));                     \
    throw std::runtime_error(                               \
        ERROR_MSG((expected), tt_to_string(__t.type) + (    \
                (get_tvt(&__t) == tvt::STRING) ?            \
                "[val: " + get_tstr_val(&__t) + "]" :       \
                ""                                          \
            ))                                              \
        );                                                  \
} while(0)

class parser_type_error: public std::exception
{
public:
    parser_type_error(token_type _expected, token_type _got, int _line) :
        expected(_expected), got(_got), line(_line)
    {}
    std::string to_string() {
        return "ERROR: expected " + tt_to_string(expected) +
            " got " + tt_to_string(got) +
            ". Line: " + std::to_string(line) + ".\n";
    }
    token_type expected, got;
    int line;
};

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

static ast_nt ttype_to_atype(token_type type);

static as_tree *handler_decl(parser *p);
static as_tree *library_decl(parser *p);
static as_tree *var_or_func_decl(parser *p);
static as_tree *var_decl(parser *p);
static as_tree *var_dev_without_init(parser *p);
static as_tree *var_dev_with_init(parser *p);
static as_tree *data(parser *p);
static as_tree *arr_init(parser *p);
static as_tree *arg_list(parser *p);
static as_tree *expr(parser *p);
static as_tree *func_decl(parser *p);
static as_tree *param_list(parser *p);
static as_tree *block_st(parser *p);

static as_tree *prog(parser *p);

static bool ebnf_multiple(parser *p, ast_children *nodes, grammar_rule rule);
static as_tree * ebnf_select(parser *p, std::vector<grammar_rule> rules);


as_tree *buid_tree(std::vector<token> &token_sequence) {
    try {
        parser p = {
            .tseq = &token_sequence,
            .it = token_sequence.cbegin(),
            .level = 0,
        };

        return prog(&p);
    }
    catch(std::exception &e) {

        return nullptr;
    }
}

void release_tree(as_tree *tree) {

}


static as_tree *prog(parser *p) {
    as_tree *prog_node = new as_tree(new ast_node(ast_nt::PROGRAM));
    ebnf_multiple(p, &prog_node->children, handler_decl);
    ebnf_multiple(p, &prog_node->children, library_decl);
    ebnf_multiple(p, &prog_node->children, var_or_func_decl);
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
	if (t.type == ttype) {
		ast_nt atype = ttype_to_atype(ttype);
		ast_node *node = (get_tvt(&t) == tvt::STRING) ?
            new ast_node(atype, get_tstr_val(&t)) :
            new ast_node(atype);

		as_tree *tree = new as_tree(node);
        inc_it(p);
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

static bool ebnf_multiple(parser *p, ast_children *children, grammar_rule rule) {
    as_tree *node = nullptr;
    while(node = rule(p), node) {
        children->push_back(node);
    }
    return true;
}

static as_tree * ebnf_select(parser *p, std::vector<grammar_rule> rules) {
    as_tree *node = nullptr;
    for (grammar_rule &rule : rules) {
        node = rule(p);
        if (node) {
            return node;
        }
    }
    return nullptr;
}

static inline as_tree *__hl_decl(parser *p, token_type ttype) {
    as_tree *main_node = accept(p, ttype);
    if (!main_node) {
        return nullptr;
    }

    pick_up(p, main_node, token_type::ID);
    bool_expect(p, token_type::FROM);
    pick_up(p, main_node, token_type::ID); //file_name
    bool_expect(p, token_type::SEMICOLON);
    return main_node;
}

static as_tree *handler_decl(parser *p) {
    return __hl_decl(p, token_type::HANDLER);
}

static as_tree *library_decl(parser *p) {
    return __hl_decl(p, token_type::LIBRARY);
}

static as_tree *var_or_func_decl(parser *p) {
    return ebnf_select(p, {var_decl, func_decl});
}

static as_tree *var_decl(parser *p) {
    return ebnf_select(p, {var_dev_without_init, var_dev_with_init});
}

static as_tree *var_dev_without_init(parser *p) {
    as_tree *main_node = accept(p, token_type::LET);
    if (!main_node) {
        return nullptr;
    }

    pick_up(p, main_node, token_type::ID);
    return main_node;
}

static as_tree *var_dev_with_init(parser *p) {
    as_tree *main_node = accept(p, token_type::LET);
    if (!main_node) {
        return nullptr;
    }

    pick_up(p, main_node, token_type::ID);
    bool_expect(p, token_type::EQUAL);
    as_tree *node = data(p);
    main_node->add_child(node);
    bool_expect(p, token_type::SEMICOLON);
    return main_node;
}

static as_tree *data(parser *p) {
    return ebnf_select(p, {arr_init, expr});
}

static as_tree *arr_init(parser *p) {
    if (!bool_accept(p, token_type::LEFT_SQUARE_BRACKET)) {
        return nullptr;
    }
    as_tree *node = arg_list(p);
    bool_expect(p, token_type::RIGHT_SQUARE_BRACKET);
    return node;
}

static as_tree *arg_list(parser *p) {
    as_tree *expr_node = expr(p);
    if (!expr_node) {
        return nullptr;
    }
    as_tree *node = new as_tree(new ast_node(ast_nt::ARG_LIST));
    if (expr_node) {
        node->add_child(expr_node);
        while(true) {
            if (!bool_accept(p, token_type::COMMA)) {
                break;
            }
            expr_node = expr(p);
            if (expr_node) {
                node->add_child(expr_node);
            } else {
                throw_error(p, "expr");
            }
        }
    }
    return node;
}

static as_tree *expr(parser *p) {
    return nullptr;
}

static as_tree *func_decl(parser *p) {
    as_tree *main_node = accept(p, token_type::FN);
    if (!main_node) {
        return nullptr;
    }
    pick_up(p, main_node, token_type::ID);
    bool_expect(p, token_type::LEFT_BRACKET);
    as_tree *node = param_list(p);
    if (node) {
        main_node->add_child(node);
    }
    bool_expect(p, token_type::RIGHT_BRACKET);
    node = block_st(p);
    if (node) {
        main_node->add_child(node);
    } else {
        throw_error(p, "block");
    }
    return main_node;
}

static as_tree *param_list(parser *p) {
    as_tree *id_node = accept(p, token_type::ID);
    if (!id_node) {
        return nullptr;
    }
    as_tree *node = new as_tree(new ast_node(ast_nt::PARAM_LIST));
    if (id_node) {
        node->add_child(id_node);
        while(true) {
            if (!bool_accept(p, token_type::COMMA)) {
                break;
            }
            pick_up(p, node, token_type::ID);
        }
    }
    return node;
}

static as_tree *block_st(parser *p) {
    return nullptr;
}



static ast_nt ttype_to_atype(token_type type) {
    return (ast_nt)0;
}