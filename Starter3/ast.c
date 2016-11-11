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
  node *n = (node *) malloc(sizeof(node));
  memset(n, 0, sizeof *n);
  n->kind = kind;

  va_start(args, kind);

  switch (kind) {
  case SCOPE_NODE:
    n->scope.declarations = va_arg(args, node *);
    n->scope.statements = va_arg(args, node *);
    break;

  case DECLARATION_NODE:
    n->declaration.is_const = va_arg(args, int);
    n->declaration.type = va_arg(args, node *);
    n->declaration.id = va_arg(args, char *);
    n->declaration.assignment_expr = va_arg(args, node *);
    break;

  case STATEMENT_NODE:
    // STATEMENT_NODE is an abstract node
    break;
  case IF_STATEMENT_NODE:
    n->statement.if_else_statement.condition = va_arg(args, node *);
    n->statement.if_else_statement.if_statement = va_arg(args, node *);
    n->statement.if_else_statement.else_statement = va_arg(args, node *);
    break;
  case ASSIGNMENT_NODE:
    n->statement.assignment.variable = va_arg(args, node *);
    n->statement.assignment.expression = va_arg(args, node *);
    break;
  case NESTED_SCOPE_NODE:
    // TODO: needed?
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    n->expression.unary.op = (unary_op) va_arg(args, int);
    n->expression.unary.right = va_arg(args, node *);
    break;
  case BINARY_EXPRESSION_NODE:
    n->expression.binary.op = (binary_op) va_arg(args, int);
    n->expression.binary.left = va_arg(args, node *);
    n->expression.binary.right = va_arg(args, node *);
    break;
  case INT_NODE:
    n->expression.int_expr.val = va_arg(args, int);
    break;
  case FLOAT_NODE:
    n->expression.float_expr.val = va_arg(args, double);
    break;
  case IDENT_NODE:
    n->expression.ident.val = va_arg(args, char *);
    break;
  case VAR_NODE:
    n->expression.variable.identifier = va_arg(args, char *);
    n->expression.variable.has_index = va_arg(args, int);
    n->expression.variable.index = va_arg(args, int);
    break;
  case FUNCTION_NODE:
    n->expression.function.func_id = va_arg(args, int);
    n->expression.function.arguments = va_arg(args, node *);
    break;
  case CONSTRUCTOR_NODE:
    n->expression.constructor.type = va_arg(args, node *);
    n->expression.constructor.arguments = va_arg(args, node *);
    break;

  case TYPE_NODE:
    n->type.type = (type_enum) va_arg(args, int);
    n->type.vec_dim = va_arg(args, int);
    break;

  case ARGUMENT_NODE:
    n->argument.expression = va_arg(args, node *);
    break;

  default: break;
  }

  va_end(args);

  return n;
}

void freeer(node *n, void *data) {
  free(n);
}

void ast_free(node *n) {
  ast_visit_postorder(n, freeer, NULL);
}

void ast_print(node *n) {
  if (n == NULL) {
    return;
  }

  switch (n->kind) {
  case SCOPE_NODE:
    printf("( SCOPE ");

    printf("( DECLARATIONS ");
    ast_print(n->scope.declarations);
    printf(" ) ");

    printf("( STATEMENTS ");
    ast_print(n->scope.statements);
    printf(" )");

    printf(" )");
    break;

  case DECLARATION_NODE:
    printf("( DECLARATION %s", n->declaration.id);
    ast_print(n->declaration.type);
    ast_print(n->declaration.assignment_expr);
    printf(" )");
    break;

  case STATEMENT_NODE:
    // STATEMENT_NODE is an abstract node
    break;
  case IF_STATEMENT_NODE:
    break;
  case ASSIGNMENT_NODE:
    break;
  case NESTED_SCOPE_NODE:
    // TODO: needed?
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    printf("( UNARY ");
    // TODO: Need to look up the types of variables using the symbol table
    break;
  case BINARY_EXPRESSION_NODE:
    // TODO: Need to look up the types of variables using the symbol table
    break;
  case INT_NODE:
    printf(" %d", n->expression.int_expr.val);
    break;
  case FLOAT_NODE:
    printf(" %f", n->expression.float_expr.val);
    break;
  case IDENT_NODE:
    printf(" %s", n->expression.ident.val);
    break;
  case VAR_NODE:
    break;
  case FUNCTION_NODE:
    printf("( CALL");

    switch (n->expression.function.func_id) {
      case FUNC_DP3:
        printf(" dp3");
        break;
      case FUNC_RSQ:
        printf(" rsq");
        break;
      case FUNC_LIT:
        printf(" lit");
        break;
      default: break;
    }

    ast_print(n->expression.function.arguments);

    printf(" )");
    break;
  case CONSTRUCTOR_NODE:
    printf("( CALL ");

    ast_print(n->expression.constructor.type);
    ast_print(n->expression.constructor.arguments);

    printf(" )");
    break;

  case TYPE_NODE:
    switch (n->type.type) {
    case TYPE_INT:
      printf(" int");
      break;
    case TYPE_IVEC:
      printf(" ivec%d", n->type.vec_dim);
      break;
    case TYPE_BOOL:
      printf(" bool");
      break;
    case TYPE_BVEC:
      printf(" bvec%d", n->type.vec_dim);
      break;
    case TYPE_FLOAT:
      printf(" float");
      break;
    case TYPE_VEC:
      printf(" vec%d", n->type.vec_dim);
      break;
    default: break;
    }
    break;

  case ARGUMENT_NODE:
    break;

  default: break;
  }
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

