#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"


void semantic_check(node *ast);

symbol_type validate_binary_expr_node(node *binary_node, bool log_errors = true);
symbol_type validate_unary_expr_node(node *unary_node, bool log_errors = true);
void validate_function_node(node *func_node, bool log_errors = true);
void validate_constructor_node(node *constructor_node, bool log_errors = true);
void validate_variable_index_node(node *var_node, bool log_errors = true);
void validate_declaration_assignment_node(node *decl_node, bool log_errors = true);
void validate_assignment_node(node *var_node, bool log_errors = true);

symbol_type get_binary_expr_type(node *binary_node);
symbol_type get_unary_expr_type(node *unary_node);
symbol_type get_function_return_type(node *func_node);
symbol_type get_base_type(symbol_type type);

#endif
