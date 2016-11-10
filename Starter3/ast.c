#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind);

  switch (kind) {
  case SCOPE_NODE:
    ast->scope.declarations = va_arg(args, node *);
    ast->scope.statements = va_arg(args, node *);
    break;

  case DECLARATION_NODE:
    ast->declaration.is_const = va_arg(args, int);
    ast->declaration.assignment_expr = va_arg(args, node *);
    break;

  case STATEMENT_NODE:
    // STATEMENT_NODE is an abstract node
    break;
  case IF_STATEMENT_NODE:
    ast->statement.if_else_statement.condition = va_arg(args, node *);
    ast->statement.if_else_statement.if_statement = va_arg(args, node *);
    ast->statement.if_else_statement.else_statement = va_arg(args, node *);
    break;
  case ASSIGNMENT_NODE:
    ast->statement.assignment.variable = va_arg(args, node *);
    ast->statement.assignment.expression = va_arg(args, node *);
    break;
  case NESTED_SCOPE_NODE:
    // TODO: needed?
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    ast->expression.unary.op = va_arg(args, int);
    ast->expression.unary.right = va_arg(args, node *);
    break;
  case BINARY_EXPRESSION_NODE:
    ast->expression.binary.op = va_arg(args, int);
    ast->expression.binary.left = va_arg(args, node *);
    ast->expression.binary.right = va_arg(args, node *);
    break;
  case INT_NODE:
    ast->expression.int_expr.val = va_arg(args, int);
    break;
  case FLOAT_NODE:
    ast->expression.float_expr.val = va_arg(args, double);
    break;
  case IDENT_NODE:
    ast->expression.ident.val = va_arg(args, char *);
    break;
  case VAR_NODE:
    ast->expression.variable.identifier = va_arg(args, char *);
    ast->expression.variable.has_index = va_arg(args, int);
    ast->expression.variable.index = va_arg(args, int);
    break;
  case FUNCTION_NODE:
    ast->expression.function.func_id = va_arg(args, int);
    ast->expression.function.arguments = va_arg(args, node *);
    break;
  case CONSTRUCTOR_NODE:
    ast->expression.constructor.type = va_arg(args, node *);
    ast->expression.constructor.arguments = va_arg(args, node *);
    break;

  case ARGUMENT_NODE:
    ast->argument.expression = va_arg(args, node *);
    break;

  default: break;
  }

  va_end(args);

  return ast;
}

void freeer(node *n, void *data) {
  free(n);
}

void ast_free(node *ast) {
  ast_visit_postorder(ast, freeer, NULL);
}

void printer(node *n, void *data) {
  printf("Hello, this is a node of type %d\n", n->kind);
}

void ast_print(node * ast) {
  ast_visit_preorder(ast, printer, NULL);
}

/****** VISITOR FUNCTIONS ******/
typedef enum {
  VISIT_PREORDER,
  VISIT_POSTORDER
} visit_order;

void visit(node *n, void (*f)(node *, void *), void *data, visit_order order);
void visitor(node *n, void (*f)(node *, void *), void *data, visit_order order);

void ast_visit_preorder(node *n, void (*f)(node *, void *), void *data) {
  visitor(n, f, data, VISIT_PREORDER);
}

void ast_visit_postorder(node *n, void (*f)(node *, void *), void *data) {
  visitor(n, f, data, VISIT_POSTORDER);
}

void visitor(node *n, void (*f)(node *, void *), void *data, visit_order order) {
  if (n != NULL) {
    if (order == VISIT_PREORDER) {
      f(n, data);
    }

    switch (n->kind) {
    case SCOPE_NODE:
      visitor(n->scope.declarations, f, data, order);
      visitor(n->scope.statements, f, data, order);
      break;

    case DECLARATION_NODE:
      visitor(n->declaration.assignment_expr, f, data, order);
      visitor(n->declaration.next_declaration, f, data, order);
      break;

    case STATEMENT_NODE:
      // STATEMENT_NODE is an abstract node
      break;
    case IF_STATEMENT_NODE:
      visitor(n->statement.if_else_statement.condition, f, data, order);
      visitor(n->statement.if_else_statement.if_statement, f, data, order);
      visitor(n->statement.if_else_statement.else_statement, f, data, order);
      visitor(n->statement.next_statement, f, data, order);
      break;
    case ASSIGNMENT_NODE:
      visitor(n->statement.assignment.variable, f, data, order);
      visitor(n->statement.assignment.expression, f, data, order);
      visitor(n->statement.next_statement, f, data, order);
      break;
    case NESTED_SCOPE_NODE:
      break;

    case EXPRESSION_NODE:
      // EXPRESSION_NODE is an abstract node
      break;
    case UNARY_EXPRESSION_NODE:
      visitor(n->expression.unary.right, f, data, order);
      break;
    case BINARY_EXPRESSION_NODE:
      visitor(n->expression.binary.left, f, data, order);
      visitor(n->expression.binary.right, f, data, order);
      break;
    case INT_NODE:
      // No children
      break;
    case FLOAT_NODE:
      // No children
      break;
    case IDENT_NODE:
      // No children
      break;
    case VAR_NODE:
      // No children
      break;
    case FUNCTION_NODE:
      visitor(n->expression.function.arguments, f, data, order);
      break;
    case CONSTRUCTOR_NODE:
      visitor(n->expression.constructor.arguments, f, data, order);
      break;

    case ARGUMENT_NODE:
      visitor(n->argument.expression, f, data, order);
      visitor(n->argument.next_argument, f, data, order);

    default: break;
    }

    if (order == VISIT_POSTORDER) {
      f(n, data);
    }
  }
}

