#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"


void semantic_check(node *ast);

symbol_type get_binary_expr_type(node *binary_node);
symbol_type get_unary_expr_type(node *unary_node);
symbol_type get_function_return_type(node *func_node);
symbol_type get_base_type(symbol_type type);

#endif
