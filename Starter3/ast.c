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

std::vector<unsigned int> scope_id_stack;

extern int yyline, yycolumn;

/****** BUILDING ******/
void set_parent(node *parent, node *child) {
  if (child != NULL) {
    child->parent = parent;
  }
}

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *n = (node *) malloc(sizeof(node));
  memset(n, 0, sizeof *n);
  n->kind = kind;
  n->parent = NULL;

  n->line = yyline;
  n->column = yycolumn - 1;

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

    // Make this node the parent of its children
    set_parent(n, n->scope.declarations);
    set_parent(n, n->scope.statements);
    break;

  case DECLARATIONS_NODE:
    n->declarations.declarations = new std::list<node *>();
    break;
  case DECLARATION_NODE:
    n->declaration.is_const = (bool) va_arg(args, int);
    n->declaration.type = va_arg(args, node *);
    n->declaration.identifier = va_arg(args, node *);
    n->declaration.assignment_expr = va_arg(args, node *);

    // Add the symbol that we are declaring to the symbol table
    symbol = n->declaration.identifier->expression.ident.val;
    sym_info.type = n->declaration.type->type.type;
    sym_info.read_only = n->declaration.is_const;
    sym_info.write_only = false;
    sym_info.constant = n->declaration.is_const;
    set_symbol_info(scope_id_stack.back(), symbol, sym_info);

    // Make the IDENT_NODE have the same type as this node
    n->declaration.identifier->expression.expr_type = n->declaration.type->type.type;

    // Make this node the parent of its children
    set_parent(n, n->declaration.type);
    set_parent(n, n->declaration.identifier);
    set_parent(n, n->declaration.assignment_expr);
    break;

  case STATEMENTS_NODE:
    n->statements.statements = new std::list<node *>();
    break;
  case IF_STATEMENT_NODE:
    n->statement.if_else_statement.condition = va_arg(args, node *);
    n->statement.if_else_statement.if_statement = va_arg(args, node *);
    n->statement.if_else_statement.else_statement = va_arg(args, node *);

    // Make this node the parent of its children
    set_parent(n, n->statement.if_else_statement.condition);
    set_parent(n, n->statement.if_else_statement.if_statement);
    set_parent(n, n->statement.if_else_statement.else_statement);
    break;
  case ASSIGNMENT_NODE:
    n->statement.assignment.variable = va_arg(args, node *);
    n->statement.assignment.expression = va_arg(args, node *);

    // Make this node the parent of its children
    set_parent(n, n->statement.assignment.variable);
    set_parent(n, n->statement.assignment.expression);
    break;
  case NESTED_SCOPE_NODE:
    n->statement.nested_scope.scope = va_arg(args, node *);

    // Make this node the parent of its children
    set_parent(n, n->statement.nested_scope.scope);
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    n->expression.unary.op = (unary_op) va_arg(args, int);
    n->expression.unary.right = va_arg(args, node *);

    n->expression.expr_type = get_unary_expr_type(n);

    // Make this node the parent of its children
    set_parent(n, n->expression.unary.right);
    break;
  case BINARY_EXPRESSION_NODE:
    n->expression.binary.op = (binary_op) va_arg(args, int);
    n->expression.binary.left = va_arg(args, node *);
    n->expression.binary.right = va_arg(args, node *);

    n->expression.expr_type = get_binary_expr_type(n);

    // Make this node the parent of its children
    set_parent(n, n->expression.binary.left);
    set_parent(n, n->expression.binary.right);
    break;
  case INT_NODE:
    n->expression.int_expr.val = va_arg(args, int);
    n->expression.expr_type = TYPE_INT;

    // This node doesn't have any children
    break;
  case FLOAT_NODE:
    n->expression.float_expr.val = va_arg(args, double);
    n->expression.expr_type = TYPE_FLOAT;

    // This node doesn't have any children
    break;
  case BOOL_NODE:
    n->expression.bool_expr.val = (bool) va_arg(args, int);
    n->expression.expr_type = TYPE_BOOL;

    // This node doesn't have any children
    break;
  case IDENT_NODE:
    n->expression.ident.val = va_arg(args, char *);

    // Look up the type of the symbol from the symbol table. If the symbol doesn't exist,
    // this will create an entry for the symbol in the symbol table with TYPE_UNKNOWN.
    // This is done so that we can find as many errors as possible.
    sym_info = get_symbol_info(scope_id_stack, n->expression.ident.val);
    n->expression.expr_type = sym_info.type;

    // This node doesn't have any children
    break;
  case VAR_NODE:
    n->expression.variable.identifier = va_arg(args, node *);
    n->expression.variable.index = va_arg(args, node *);

    if (n->expression.variable.index == NULL) {
      // Copy the type from the identifier directly since we aren't indexing
      n->expression.expr_type = n->expression.variable.identifier->expression.expr_type;
    } else {
      // Get the base type since we are indexing
      n->expression.expr_type = get_base_type(n->expression.variable.identifier->expression.expr_type);
    }

    // Make this node the parent of its children
    set_parent(n, n->expression.variable.identifier);
    set_parent(n, n->expression.variable.index);
    break;
  case FUNCTION_NODE:
    n->expression.function.func_id = (function_id) va_arg(args, int);
    n->expression.function.arguments = va_arg(args, node *);

    // Look up the return type of the function
    n->expression.expr_type = get_function_return_type(n);

    // Make this node the parent of its children
    set_parent(n, n->expression.function.arguments);
    break;
  case CONSTRUCTOR_NODE:
    n->expression.constructor.type = va_arg(args, node *);
    n->expression.constructor.arguments = va_arg(args, node *);

    n->expression.expr_type = n->expression.constructor.type->type.type;

    // Make this node the parent of its children
    set_parent(n, n->expression.constructor.type);
    set_parent(n, n->expression.constructor.arguments);
    break;

  case TYPE_NODE:
    n->type.type = (symbol_type) va_arg(args, int);
    break;

  case ARGUMENT_NODE:
    n->argument.expression = va_arg(args, node *);
    n->argument.next_argument = NULL;
    n->argument.num_arguments = 1;

    // Points to the last argument seen. Only valid for the first argument
    // and only used during construction.
    n->argument.last_argument = n;

    // Make this node the parent of its children
    set_parent(n, n->argument.expression);
    break;

  default: break;
  }

  va_end(args);

  return n;
}

/****** FREEING ******/
void free_postorder(node *n, void *data) {
  switch (n->kind) {
  case DECLARATIONS_NODE:
    delete n->declarations.declarations;
    break;
  case STATEMENTS_NODE:
    delete n->statements.statements;
    break;
  case IDENT_NODE:
    free(n->expression.ident.val);
    break;
  default:
    break;
  }
  free(n);
}

void ast_free(node *n) {
  ast_visit(n, NULL, free_postorder, NULL);
}

/****** PRINTING ******/
#define PRINT_AST(fmt, ...) { fprintf(dumpFile, fmt, ##__VA_ARGS__); }

void print_preorder(node *n, void *data) {
  std::vector<unsigned int> *scope_id_stack = (std::vector<unsigned int> *) data;

  switch (n->kind) {
  case SCOPE_NODE:
    scope_id_stack->push_back(n->scope.scope_id);
    PRINT_AST(" (SCOPE");
    break;

  case DECLARATIONS_NODE:
    PRINT_AST(" (DECLARATIONS");
    break;
  case DECLARATION_NODE:
    PRINT_AST(" (DECLARATION");
    break;

  case STATEMENTS_NODE:
    PRINT_AST(" (STATEMENTS");
    break;
  case IF_STATEMENT_NODE:
    PRINT_AST(" (IF");
    break;
  case ASSIGNMENT_NODE:
    PRINT_AST(" (ASSIGN ");
    PRINT_AST("%s", get_type_name(n->statement.assignment.variable->expression.expr_type));
    break;
  case NESTED_SCOPE_NODE:
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    PRINT_AST(" (UNARY ");
    PRINT_AST("%s", get_type_name(get_unary_expr_type(n)));
    PRINT_AST(" ");
    PRINT_AST("%s", get_unary_op_name(n->expression.unary.op));
    break;
  case BINARY_EXPRESSION_NODE:
    PRINT_AST(" (BINARY ");
    PRINT_AST("%s", get_type_name(get_binary_expr_type(n)));
    PRINT_AST(" ");
    PRINT_AST("%s", get_binary_op_name(n->expression.binary.op));
    break;
  case INT_NODE:
    PRINT_AST(" %d", n->expression.int_expr.val);
    break;
  case FLOAT_NODE:
    PRINT_AST(" %f", n->expression.float_expr.val);
    break;
  case BOOL_NODE:
    PRINT_AST(" ");
    PRINT_AST(n->expression.bool_expr.val ? "true" : "false");
    break;
  case IDENT_NODE:
    PRINT_AST(" %s", n->expression.ident.val);
    break;
  case VAR_NODE:
    if (n->expression.variable.index != NULL) {
      PRINT_AST(" (INDEX ");
      PRINT_AST("%s", get_type_name(n->expression.expr_type));
    }
    break;
  case FUNCTION_NODE:
    PRINT_AST(" (CALL ");
    PRINT_AST("%s", get_function_name(n->expression.function.func_id));
    break;
  case CONSTRUCTOR_NODE:
    PRINT_AST(" (CALL");
    break;

  case TYPE_NODE:
    PRINT_AST(" ");
    PRINT_AST("%s", get_type_name(n->type.type));
    break;

  case ARGUMENT_NODE:
    // Don't need to print anything special for the arguments, just
    // let the visitor print their children.
    break;

  default: break;
  }
}

void print_postorder(node *n, void *data) {
  std::vector<unsigned int> *scope_id_stack = (std::vector<unsigned int> *) data;
  switch (n->kind) {
  case SCOPE_NODE:
    PRINT_AST(")");
    scope_id_stack->pop_back();
    break;

  case DECLARATIONS_NODE:
    PRINT_AST(")");
    break;
  case DECLARATION_NODE:
    PRINT_AST(")");
    break;

  case STATEMENTS_NODE:
    PRINT_AST(")");
    break;
  case IF_STATEMENT_NODE:
    PRINT_AST(")");
    break;
  case ASSIGNMENT_NODE:
    PRINT_AST(")");
    break;
  case NESTED_SCOPE_NODE:
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    PRINT_AST(")");
    break;
  case BINARY_EXPRESSION_NODE:
    PRINT_AST(")");
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
      PRINT_AST(")");
    }
    break;
  case FUNCTION_NODE:
    PRINT_AST(")");
    break;
  case CONSTRUCTOR_NODE:
    PRINT_AST(")");
    break;

  case TYPE_NODE:
    break;

  case ARGUMENT_NODE:
    // Don't need to print anything special for the arguments, just
    // let the visitor print their children.
    break;

  default: break;
  }
}

void ast_print(node *n) {
  std::vector<unsigned int> scope_id_stack;
  scope_id_stack.push_back(0);

  ast_visit(n, print_preorder, print_postorder, &scope_id_stack);
  PRINT_AST("\n");
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

    std::list<node *>::iterator iter;

    switch (n->kind) {
    case SCOPE_NODE:
      ast_visit(n->scope.declarations, preorder, postorder, data);
      ast_visit(n->scope.statements, preorder, postorder, data);
      break;

    case DECLARATIONS_NODE:
      for (iter = n->declarations.declarations->begin(); iter != n->declarations.declarations->end(); iter++) {
        ast_visit(*iter, preorder, postorder, data);
      }
      break;
    case DECLARATION_NODE:
      ast_visit(n->declaration.identifier, preorder, postorder, data);
      ast_visit(n->declaration.type, preorder, postorder, data);
      ast_visit(n->declaration.assignment_expr, preorder, postorder, data);
      break;

    case STATEMENTS_NODE:
      for (iter = n->statements.statements->begin(); iter != n->statements.statements->end(); iter++) {
        ast_visit(*iter, preorder, postorder, data);
      }
      break;
    case IF_STATEMENT_NODE:
      ast_visit(n->statement.if_else_statement.condition, preorder, postorder, data);
      ast_visit(n->statement.if_else_statement.if_statement, preorder, postorder, data);
      ast_visit(n->statement.if_else_statement.else_statement, preorder, postorder, data);
      break;
    case ASSIGNMENT_NODE:
      ast_visit(n->statement.assignment.variable, preorder, postorder, data);
      ast_visit(n->statement.assignment.expression, preorder, postorder, data);
      break;
    case NESTED_SCOPE_NODE:
      ast_visit(n->statement.nested_scope.scope, preorder, postorder, data);
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

const char *get_function_name(function_id func_id) {
  switch (func_id) {
  case FUNC_DP3: return "dp3"; break;
  case FUNC_RSQ: return "rsq"; break;
  case FUNC_LIT: return "lit"; break;
  }
  return "unknown";
}

const char *get_type_name(symbol_type type) {
  switch (type) {
  case TYPE_INT:     return "int";
  case TYPE_BOOL:    return "bool";
  case TYPE_FLOAT:   return "float";
  case TYPE_VEC2:    return "vec2";
  case TYPE_VEC3:    return "vec3";
  case TYPE_VEC4:    return "vec4";
  case TYPE_IVEC2:   return "ivec2";
  case TYPE_IVEC3:   return "ivec3";
  case TYPE_IVEC4:   return "ivec4";
  case TYPE_BVEC2:   return "bvec2";
  case TYPE_BVEC3:   return "bvec3";
  case TYPE_BVEC4:   return "bvec4";
  default: break;
  }
  return "unknown";
}

const char *get_unary_op_name(unary_op op) {
  switch (op) {
  case OP_UMINUS: return "-";
  case OP_NOT: return "!";
  }
  return "unknown";
}

const char *get_binary_op_name(binary_op op) {
  switch (op) {
  case OP_AND:    return "&&";
  case OP_OR:     return "||";
  case OP_EQ:     return "==";
  case OP_NEQ:    return "!=";
  case OP_LT:     return "<";
  case OP_LEQ:    return "<=";
  case OP_GT:     return ">";
  case OP_GEQ:    return ">=";
  case OP_PLUS:   return "+";
  case OP_MINUS:  return "-";
  case OP_MUL:    return "*";
  case OP_DIV:    return "/";
  case OP_XOR:    return "^";
  }
  return "unknown";
}

