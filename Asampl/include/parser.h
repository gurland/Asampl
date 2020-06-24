#pragma once
#include "lexer.h"
#include "tree.h"


Tree * parser_buid_tree(std::vector<Lexem>* lexem_sequence);

