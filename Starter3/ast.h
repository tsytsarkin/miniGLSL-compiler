
#ifndef AST_H_
#define AST_H_ 1

#include <stdarg.h>

// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
typedef struct node_ node;
extern node *ast;

typedef enum {
  UNKNOWN                = 0,

  SCOPE_NODE             = (1 << 0),
 
  EXPRESSION_NODE        = (1 << 2),
  UNARY_EXPRESSION_NODE  = (1 << 2) | (1 << 3),
  BINARY_EXPRESSION_NODE = (1 << 2) | (1 << 4),
  INT_NODE               = (1 << 2) | (1 << 5),
  FLOAT_NODE             = (1 << 2) | (1 << 6),
  IDENT_NODE             = (1 << 2) | (1 << 7),
  VAR_NODE               = (1 << 2) | (1 << 8),
  FUNCTION_NODE          = (1 << 2) | (1 << 9),
  CONSTRUCTOR_NODE       = (1 << 2) | (1 << 10),

  STATEMENT_NODE         = (1 << 1),
  IF_STATEMENT_NODE      = (1 << 1) | (1 << 11),
  ASSIGNMENT_NODE        = (1 << 1) | (1 << 12),
  NESTED_SCOPE_NODE      = (1 << 1) | (1 << 13),

  DECLARATION_NODE       = (1 << 14),

  TYPE_NODE              = (1 << 15),

  ARGUMENT_NODE         = (1 << 16),
} node_kind;

typedef enum {
  OP_UMINUS,
  OP_NOT
} unary_op;

typedef enum {
  OP_AND,
  OP_OR,
  OP_EQ,
  OP_NEQ,
  OP_LT,
  OP_LEQ,
  OP_GT,
  OP_GEQ,
  OP_PLUS,
  OP_MINUS,
  OP_MUL,
  OP_DIV,
  OP_XOR
} binary_op;

struct node_ {

  // an example of tagging each node with a type
  node_kind kind;

  union {
    struct {
      node *declarations;
      node *statements;
    } scope;

    struct {
      bool is_const;
      node *type;
      char *id;
      node *assignment_expr;
      node *next_declaration;
    } declaration;

    struct {
      union {
        struct {
          node *condition;
          node *if_statement;
          node *else_statement;
        } if_else_statement;

        struct {
          node *variable;
          node *expression;
        } assignment;

        // TODO: is this really needed?
        struct {
        } nested_scope;
      };

      node *next_statement;
    } statement;

    struct {
      union {
        struct {
          int op;
          node *right;
        } unary;

        struct {
          int op;
          node *left;
          node *right;
        } binary;

        struct {
          int val;
        } int_expr;

        struct {
          float val;
        } float_expr;

        struct {
          char *val;
        } ident;

        struct {
          char *identifier;
          bool has_index;
          int index;
        } variable;

        struct {
          int func_id;
          node *arguments;
        } function;

        struct {
          node *type;
          node *arguments;
        } constructor;
      };
    } expression;


    struct {
      node *expression;
      node *next_argument;
    } argument;
  };
};

node *ast_allocate(node_kind type, ...);
void ast_free(node *ast);
void ast_print(node * ast);

void ast_visit_preorder(node *root, void (*f)(node *, void *), void *data);
void ast_visit_postorder(node *root, void (*f)(node *, void *), void *data);

#endif /* AST_H_ */
