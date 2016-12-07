#include "codegen.h"
#include "common.h"
#include <vector>
#include <map>
#include <string>

#define INSTRUCTION(instr, fmt, ...) { \
  fprintf(outputFile, instr " " fmt ";\n", ##__VA_ARGS__); \
}

// Map variables to registers
std::vector<std::map<std::string, std::string> > register_tables;

// Map expression nodes to intermediate register number
std::map<node *, unsigned int> intermediate_registers;

typedef struct {
  std::vector<unsigned int> scope_id_stack;
} visit_data;

void generate_assignment_code(node *assign);

void codegen_preorder(node *n, void *data) {
  visit_data *vd = (visit_data *) data;

  char *str;

  switch (n->kind) {
  case SCOPE_NODE:
    vd->scope_id_stack.push_back(n->scope.scope_id);
    register_tables.push_back(std::map<std::string, std::string>());
    break;

  case DECLARATIONS_NODE:
    break;
  case DECLARATION_NODE:
    str = n->declaration.identifier->expression.ident.val;
    // Assign this variable to the corresponding register
    register_tables[vd->scope_id_stack.back()][str] = str;

    INSTRUCTION("TEMP", "%s", str);
    break;

  case STATEMENTS_NODE:
    break;
  case IF_STATEMENT_NODE:
    break;
  case ASSIGNMENT_NODE:
    generate_assignment_code(n);
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

void generate_assignment_code(node *assign) {
  std::map<node *, unsigned int>::iterator iter;
  iter = intermediate_registers.find(assign->statement.assignment.expression);
  if (iter != intermediate_registers.end()) {
    // TODO: handle the case where the variable is indexed
    char *var_name = assign->statement.assignment.variable->expression.variable.identifier->expression.ident.val;
    INSTRUCTION("MOV", "%s, tempVar%d", var_name, iter->second);
  }
}

