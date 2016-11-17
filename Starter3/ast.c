#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "ast.h"
#include "symbol.h"
#include "semantic.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

std::vector<int> scope_id_stack;

/****** BUILDING ******/
node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *n = (node *) malloc(sizeof(node));
  memset(n, 0, sizeof *n);
  n->kind = kind;

  va_start(args, kind);

  char *symbol;
  symbol_info sym_info;

  switch (kind) {
  case SCOPE_NODE:
    n->scope.declarations = va_arg(args, node *);
    n->scope.statements = va_arg(args, node *);

    // When we are creating a scope node, we have already built all of its children
    // so its id should be the current scope id
    n->scope.scope_id = scope_id_stack.back();
    break;

  case DECLARATIONS_NODE:
    n->declarations.first_declaration = NULL;
    n->declarations.last_declaration = NULL;
    break;
  case DECLARATION_NODE:
    n->declaration.is_const = (bool) va_arg(args, int);
    n->declaration.type = va_arg(args, node *);
    n->declaration.identifier = va_arg(args, node *);
    n->declaration.assignment_expr = va_arg(args, node *);
    n->declaration.next_declaration = NULL;

    // Add the symbol that we are declaring to the symbol table
    symbol = n->declaration.identifier->expression.ident.val;
    sym_info.type = n->declaration.type->type.type;
    symbol_tables[scope_id_stack.back()].insert(std::make_pair(symbol, sym_info));
    break;

  case STATEMENTS_NODE:
    n->statements.first_statement = NULL;
    n->statements.last_statement = NULL;
    break;
  case IF_STATEMENT_NODE:
    n->statement.if_else_statement.condition = va_arg(args, node *);
    n->statement.if_else_statement.if_statement = va_arg(args, node *);
    n->statement.if_else_statement.else_statement = va_arg(args, node *);
    n->statement.next_statement = NULL;
    break;
  case ASSIGNMENT_NODE:
    n->statement.assignment.variable = va_arg(args, node *);
    n->statement.assignment.expression = va_arg(args, node *);
    n->statement.next_statement = NULL;
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    n->expression.unary.op = (unary_op) va_arg(args, int);
    n->expression.unary.right = va_arg(args, node *);

    n->expression.expr_type = get_unary_expr_type(n);
    break;
  case BINARY_EXPRESSION_NODE:
    n->expression.binary.op = (binary_op) va_arg(args, int);
    n->expression.binary.left = va_arg(args, node *);
    n->expression.binary.right = va_arg(args, node *);

    n->expression.expr_type = get_binary_expr_type(n);
    break;
  case INT_NODE:
    n->expression.int_expr.val = va_arg(args, int);
    n->expression.expr_type = TYPE_INT;
    break;
  case FLOAT_NODE:
    n->expression.float_expr.val = va_arg(args, double);
    n->expression.expr_type = TYPE_FLOAT;
    break;
  case BOOL_NODE:
    n->expression.bool_expr.val = (bool) va_arg(args, int);
    n->expression.expr_type = TYPE_BOOL;
    break;
  case IDENT_NODE:
    n->expression.ident.val = va_arg(args, char *);

    // Look up the type of the symbol from the symbol table. If the symbol doesn't exist,
    // this will create an entry for the symbol in the symbol table with TYPE_UNKNOWN.
    // This is done so that we can find as many errors as possible.
    sym_info = get_symbol_info(scope_id_stack, n->expression.variable.identifier->expression.ident.val);
    n->expression.expr_type = sym_info.type;
    break;
  case VAR_NODE:
    n->expression.variable.identifier = va_arg(args, node *);
    n->expression.variable.index = va_arg(args, node *);

    if (n->expression.variable.index != NULL) {
      // Copy the type from the identifier directly since we aren't indexing
      n->expression.expr_type = n->expression.variable.identifier->expression.expr_type;
    } else {
      // Get the base type since we are indexing
      n->expression.expr_type = get_base_type(n->expression.variable.identifier->expression.expr_type);
    }
    break;
  case FUNCTION_NODE:
    n->expression.function.func_id = (function_id) va_arg(args, int);
    n->expression.function.arguments = va_arg(args, node *);

    // Look up the return type of the function
    n->expression.expr_type = get_function_return_type(n);
    break;
  case CONSTRUCTOR_NODE:
    n->expression.constructor.type = va_arg(args, node *);
    n->expression.constructor.arguments = va_arg(args, node *);

    n->expression.expr_type = n->expression.constructor.type->type.type;
    break;

  case TYPE_NODE:
    n->type.type = (symbol_type) va_arg(args, int);
    break;

  case ARGUMENT_NODE:
    n->argument.expression = va_arg(args, node *);
    n->argument.next_argument = NULL;

    // Points to the last argument seen. Only valid for the first argument
    // and only used during construction.
    n->argument.last_argument = n;
    break;

  default: break;
  }

  va_end(args);

  return n;
}

/****** FREEING ******/
void free_postorder(node *n, void *data) {
  free(n);
}

void ast_free(node *n) {
  ast_visit(n, NULL, free_postorder, NULL);
}

/****** PRINTING ******/
void print_function_name(function_id func_id) {
  switch (func_id) {
  case FUNC_DP3: printf("dp3"); break;
  case FUNC_RSQ: printf("rsq"); break;
  case FUNC_LIT: printf("lit"); break;
  }
}

void print_type_name(symbol_type type) {
  if (type & TYPE_VEC) {
    printf("vec%d", type - TYPE_VEC);
  } else if (type & TYPE_IVEC) {
    printf("ivec%d", type - TYPE_IVEC);
  } else if (type & TYPE_BVEC) {
    printf("bvec%d", type - TYPE_BVEC);
  } else {
    switch (type) {
    case TYPE_INT:   printf("int"); break;
    case TYPE_BOOL:  printf("bool"); break;
    case TYPE_FLOAT: printf("float"); break;
    default: break;
    }
  }
}

void print_preorder(node *n, void *data) {
  std::vector<int> *scope_id_stack = (std::vector<int> *) data;

  switch (n->kind) {
  case SCOPE_NODE:
    scope_id_stack->push_back(n->scope.scope_id);
    printf("(SCOPE ");
    break;

  case DECLARATIONS_NODE:
    printf("(DECLARATIONS ");
    break;
  case DECLARATION_NODE:
    printf("(DECLARATION ");
    break;

  case STATEMENTS_NODE:
    printf("(STATEMENTS ");
    break;
  case IF_STATEMENT_NODE:
    printf("(IF ");
    break;
  case ASSIGNMENT_NODE:
    printf("(ASSIGN ");
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    printf("(UNARY ");
    // TODO: Put the type of an expression in the AST (and populate it partially during build time (constants) and during semantic analysis)
    break;
  case BINARY_EXPRESSION_NODE:
    // TODO: Put the type of an expression in the AST (and populate it partially during build time (constants) and during semantic analysis)
    break;
  case INT_NODE:
    printf("%d ", n->expression.int_expr.val);
    break;
  case FLOAT_NODE:
    printf("%f ", n->expression.float_expr.val);
    break;
  case BOOL_NODE:
    printf(n->expression.bool_expr.val ? "true" : "false");
    break;
  case IDENT_NODE:
    printf("%s ", n->expression.ident.val);
    break;
  case VAR_NODE:
    if (n->expression.variable.index != NULL) {
      printf("(INDEX ");
      print_type_name(n->expression.expr_type);
      printf(" ");
    }
    break;
  case FUNCTION_NODE:
    printf("(CALL ");
    print_function_name(n->expression.function.func_id);
    break;
  case CONSTRUCTOR_NODE:
    printf("(CALL ");
    break;

  case TYPE_NODE:
    print_type_name(n->type.type);
    printf(" ");
    break;

  case ARGUMENT_NODE:
    break;

  default: break;
  }
}

void print_postorder(node *n, void *data) {
  std::vector<int> *scope_id_stack = (std::vector<int> *) data;

  switch (n->kind) {
  case SCOPE_NODE:
    printf(") ");
    scope_id_stack->pop_back();
    break;

  case DECLARATIONS_NODE:
    printf(") ");
    break;
  case DECLARATION_NODE:
    printf(") ");
    break;

  case STATEMENTS_NODE:
    printf(") ");
    break;
  case IF_STATEMENT_NODE:
    printf(") ");
    break;
  case ASSIGNMENT_NODE:
    printf(") ");
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    printf(") ");
    break;
  case BINARY_EXPRESSION_NODE:
    printf(") ");
    break;
  case INT_NODE:
    break;
  case FLOAT_NODE:
    break;
  case BOOL_NODE:
    break;
  case IDENT_NODE:
    break;
  case VAR_NODE:
    if (n->expression.variable.index != NULL) {
      printf(") ");
    }
    break;
  case FUNCTION_NODE:
    printf(") ");
    break;
  case CONSTRUCTOR_NODE:
    printf(") ");
    break;

  case TYPE_NODE:
    break;

  case ARGUMENT_NODE:
    break;

  default: break;
  }
}

void ast_print(node *n) {
  std::vector<int> scope_id_stack;
  scope_id_stack.push_back(0);

  ast_visit(n, print_preorder, print_postorder, &scope_id_stack);
  printf("\n");
}

/****** VISITOR ******/
void ast_visit(node *n,
               void (*preorder)(node *, void *),
               void (*postorder)(node *, void *),
               void *data) {
  if (n != NULL) {
    if (preorder != NULL) {
      preorder(n, data);
    }

    switch (n->kind) {
    case SCOPE_NODE:
      ast_visit(n->scope.declarations, preorder, postorder, data);
      ast_visit(n->scope.statements, preorder, postorder, data);
      break;

    case DECLARATIONS_NODE:
      ast_visit(n->declarations.first_declaration, preorder, postorder, data);
      break;
    case DECLARATION_NODE:
      ast_visit(n->declaration.identifier, preorder, postorder, data);
      ast_visit(n->declaration.type, preorder, postorder, data);
      ast_visit(n->declaration.assignment_expr, preorder, postorder, data);
      ast_visit(n->declaration.next_declaration, preorder, postorder, data);
      break;

    case STATEMENTS_NODE:
      ast_visit(n->statements.first_statement, preorder, postorder, data);
      break;
    case IF_STATEMENT_NODE:
      ast_visit(n->statement.if_else_statement.condition, preorder, postorder, data);
      ast_visit(n->statement.if_else_statement.if_statement, preorder, postorder, data);
      ast_visit(n->statement.if_else_statement.else_statement, preorder, postorder, data);
      ast_visit(n->statement.next_statement, preorder, postorder, data);
      break;
    case ASSIGNMENT_NODE:
      ast_visit(n->statement.assignment.variable, preorder, postorder, data);
      ast_visit(n->statement.assignment.expression, preorder, postorder, data);
      ast_visit(n->statement.next_statement, preorder, postorder, data);
      break;

    case EXPRESSION_NODE:
      // EXPRESSION_NODE is an abstract node
      break;
    case UNARY_EXPRESSION_NODE:
      ast_visit(n->expression.unary.right, preorder, postorder, data);
      break;
    case BINARY_EXPRESSION_NODE:
      ast_visit(n->expression.binary.left, preorder, postorder, data);
      ast_visit(n->expression.binary.right, preorder, postorder, data);
      break;
    case INT_NODE:
      // No children
      break;
    case FLOAT_NODE:
      // No children
      break;
    case BOOL_NODE:
      // No children
      break;
    case IDENT_NODE:
      // No children
      break;
    case VAR_NODE:
      ast_visit(n->expression.variable.identifier, preorder, postorder, data);
      ast_visit(n->expression.variable.index, preorder, postorder, data);
      break;
    case FUNCTION_NODE:
      ast_visit(n->expression.function.arguments, preorder, postorder, data);
      break;
    case CONSTRUCTOR_NODE:
      ast_visit(n->expression.constructor.type, preorder, postorder, data);
      ast_visit(n->expression.constructor.arguments, preorder, postorder, data);
      break;

    case TYPE_NODE:
      break;

    case ARGUMENT_NODE:
      ast_visit(n->argument.expression, preorder, postorder, data);
      ast_visit(n->argument.next_argument, preorder, postorder, data);
      break;

    default: break;
    }

    if (postorder != NULL) {
      postorder(n, data);
    }
  }
}

