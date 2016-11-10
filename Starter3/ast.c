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
  case EXPRESSION_NODE:
    // EXPRESSION_NODE is a class of nodes
    break;
  case UNARY_EXPRESSION_NODE:
    break;
  case BINARY_EXPRESSION_NODE:
    ast->binary_expr.op = va_arg(args, int);
    ast->binary_expr.left = va_arg(args, node *);
    ast->binary_expr.right = va_arg(args, node *);
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
    break;
  case FUNCTION_NODE:
    break;
  case CONSTRUCTOR_NODE:
    break;
  case STATEMENT_NODE:
    // STATEMENT_NODE is a class of nodes
    break;
  case IF_STATEMENT_NODE:
    break;
  case WHILE_STATEMENT_NODE:
    break;
  case ASSIGNMENT_NODE:
    break;
  case NESTED_SCOPE_NODE:
    break;
  case DECLARATION_NODE:
    break;
  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {

}

typedef enum {
  VISIT_PREORDER,
  VISIT_POSTORDER
} visit_order;

void visit(node *n, void (*f)(node *, void *), void *data, visit_order order);
void visitor(node *root, void (*f)(node *, void *), void *data, visit_order order);

void visit(node *n, void (*f)(node *, void *), void *data, visit_order order) {
  if (n != NULL) {
    if (order == VISIT_PREORDER) {
      f(n, data);
      visitor(n, f, data, order);
    } else if (order == VISIT_POSTORDER) {
      visitor(n, f, data, order);
      f(n, data);
    }
  }
}

void visitor(node *root, void (*f)(node *, void *), void *data, visit_order order) {
  if (root != NULL) {
    if (order == VISIT_PREORDER) {
      f(root, data);
    }

    switch (ast->kind) {
    case SCOPE_NODE:
      visit(root->scope.declarations, f, data, order);
      visit(root->scope.statements, f, data, order);
      break;
    case EXPRESSION_NODE:
      // EXPRESSION_NODE is a class of nodes
      break;
    case UNARY_EXPRESSION_NODE:
      visit(root->unary_expr.right, f, data, order);
      break;
    case BINARY_EXPRESSION_NODE:
      visit(root->binary_expr.left, f, data, order);
      visit(root->binary_expr.right, f, data, order);
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
      break;
    case FUNCTION_NODE:
      break;
    case CONSTRUCTOR_NODE:
      break;
    case STATEMENT_NODE:
      // STATEMENT_NODE is a class of nodes
      break;
    case IF_STATEMENT_NODE:
      break;
    case WHILE_STATEMENT_NODE:
      break;
    case ASSIGNMENT_NODE:
      break;
    case NESTED_SCOPE_NODE:
      break;
    case DECLARATION_NODE:
      break;
    default: break;
    }

    if (order == VISIT_POSTORDER) {
      f(root, data);
    }
  }
}

void ast_visit_preorder(node *root, void (*f)(node *, void *), void *data) {
  visitor(root, f, data, VISIT_PREORDER);
}

void ast_visit_postorder(node *root, void (*f)(node *, void *), void *data) {
  visitor(root, f, data, VISIT_POSTORDER);
}

