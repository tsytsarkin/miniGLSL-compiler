#include "codegen.h"
#include "common.h"

#define INSTRUCTION(instr, fmt, ...) { \
  fprintf(outputFile, instr " " fmt ";\n", ##__VA_ARGS__); \
}

typedef struct {
  std::vector<unsigned int> scope_id_stack;
} visit_data;

void codegen_preorder(node *n, void *data) {
  visit_data *vd = (visit_data *) data;

  switch (n->kind) {
  case SCOPE_NODE:
    vd->scope_id_stack.push_back(n->scope.scope_id);
    break;

  case DECLARATIONS_NODE:
    break;
  case DECLARATION_NODE:
    INSTRUCTION("TEMP", "%s", n->declaration.identifier->expression.ident.val);
    break;

  case STATEMENTS_NODE:
    break;
  case IF_STATEMENT_NODE:
    break;
  case ASSIGNMENT_NODE:
    break;
  case NESTED_SCOPE_NODE:
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    break;
  case BINARY_EXPRESSION_NODE:
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
    break;
  case FUNCTION_NODE:
    break;
  case CONSTRUCTOR_NODE:
    break;

  case TYPE_NODE:
    break;

  case ARGUMENT_NODE:
    break;

  default: break;
  }
}

void codegen_postorder(node *n, void *data) {
  visit_data *vd = (visit_data *) data;

  switch (n->kind) {
  case SCOPE_NODE:
    vd->scope_id_stack.pop_back();
    break;

  case DECLARATIONS_NODE:
    break;
  case DECLARATION_NODE:
    break;

  case STATEMENTS_NODE:
    break;
  case IF_STATEMENT_NODE:
    break;
  case ASSIGNMENT_NODE:
    break;
  case NESTED_SCOPE_NODE:
    break;

  case EXPRESSION_NODE:
    // EXPRESSION_NODE is an abstract node
    break;
  case UNARY_EXPRESSION_NODE:
    break;
  case BINARY_EXPRESSION_NODE:
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
    break;
  case FUNCTION_NODE:
    break;
  case CONSTRUCTOR_NODE:
    break;

  case TYPE_NODE:
    break;

  case ARGUMENT_NODE:
    break;

  default: break;
  }
}

void genCode(node *ast) {
  // Print the fragment shader header
  fprintf(outputFile, "!!ARBfp1.0\n");

  // Perform code generation
  visit_data vd;
  ast_visit(ast, codegen_preorder, codegen_postorder, &vd);

  // Print the fragment shader footer
  fprintf(outputFile, "END");
}
