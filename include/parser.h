#ifndef _PARSER_H
#define _PARSER_H
#include <vector>

#include "lexer.h"
#include "tree.h"

as_tree *buid_tree(std::vector<token> &token_sequence);
void release_tree(as_tree *tree);
std::string at_to_string(ast_nt type);
#endif /* _PARSER_H */