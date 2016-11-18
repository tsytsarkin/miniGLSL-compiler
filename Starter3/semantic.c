#include <cstdlib>
#include "semantic.h"

typedef struct {
  bool error_occurred;
} visit_data;

void semantic_preorder(node *n, void *data) {
  visit_data *vd = (visit_data *) data;

  switch (n->kind) {
  case SCOPE_NODE:
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

void semantic_postorder(node *n, void *data) {
  visit_data *vd = (visit_data *) data;

  switch (n->kind) {
  case SCOPE_NODE:
    break;

  case DECLARATIONS_NODE:
    break;
  case DECLARATION_NODE:
    break;

  case STATEMENTS_NODE:
    break;
  case IF_STATEMENT_NODE:
    // An if condition must be a boolean
    if (n->statement.if_else_statement.condition->expression.expr_type != TYPE_BOOL) {
      // TODO: log invalid condition
      vd->error_occurred = true;
    }
    break;
  case ASSIGNMENT_NODE:
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

int semantic_check(node *ast) {
  // Perform semantic analysis
  visit_data vd;
  ast_visit(ast, semantic_preorder, semantic_postorder, &vd);
  // Return 0 if error occurred
  return !vd.error_occurred;
}

/****** SEMANTIC HELPER FUNCTIONS ******/
symbol_type get_binary_expr_type(node *binary_node) {
  node *right = binary_node->expression.binary.right;
  node *left = binary_node->expression.binary.left;

  symbol_type r_type = right->expression.expr_type;
  symbol_type l_type = left->expression.expr_type;

  symbol_type r_base_type = get_base_type(r_type);
  symbol_type l_base_type = get_base_type(l_type);

  bool r_is_vec = r_type & TYPE_ANY_VEC;
  bool l_is_vec = l_type & TYPE_ANY_VEC;

  binary_op op = binary_node->expression.binary.op;

  // TODO: In alot of the cases below we can just log something and fall
  // through to the return statement at the bottom of the function. This
  // would remove alot of extra code and make it more readable

  // For a binary op, base types must match
  if (r_base_type != l_base_type) {
    // TODO: log error (type mismatch)
    return TYPE_UNKNOWN;
  }

  // Check for unknown input type
  if (l_type == TYPE_UNKNOWN || r_type == TYPE_UNKNOWN) {
    return TYPE_UNKNOWN;
  }

  switch (op) {
  case OP_AND: case OP_OR:
    // The types must be logical and equal
    if (r_type == l_type && r_base_type == TYPE_BOOL) {
      return r_type;
    } else {
      // TODO: log error (not a logical type)
      return TYPE_UNKNOWN;
    }
    break;
  case OP_PLUS: case OP_MINUS:
    // The types must be equal and arithmetic
    if (r_type == l_type && r_base_type != TYPE_BOOL) {
      return r_type;
    } else {
      // TODO: log error (not an arithmetic type)
      return TYPE_UNKNOWN;
    }
    break;
  case OP_DIV: case OP_XOR:
    // The types must be equal, non-vector and arithmetic
    if (r_type == l_type && !r_is_vec && !l_is_vec && r_base_type != TYPE_BOOL) {
      return r_type;
    } else {
      // TODO: log error. Unsupported type
      return TYPE_UNKNOWN;
    }
    break;
  case OP_MUL:
    if (r_base_type == TYPE_BOOL) {
      // TODO: log error. Unsupported type
      return TYPE_UNKNOWN;
    }
    if (l_is_vec && r_is_vec) {
      if (r_type == l_type) {
        return r_base_type;
      } else {
        // TODO: log error. trying to multiply 2 vectors with different size
        return TYPE_UNKNOWN;
      }
    } else if (l_is_vec && !r_is_vec) {
      return l_type;
    } else if (!l_is_vec && r_is_vec) {
      return r_type;
    } else /* !l_is_vec && !r_is_vec */ {
      return r_base_type;
    }
  case OP_LT: case OP_LEQ: case OP_GT: case OP_GEQ:
    // The types must be equal, non-vector and arithmetic
    if (r_type == l_type && !r_is_vec && !l_is_vec && r_base_type != TYPE_BOOL) {
        return r_type;
    } else {
      // TODO: log error. type mismatch
      return TYPE_UNKNOWN;
    }
  case OP_EQ: case OP_NEQ:
    // The types for OP_EQ and OP_NEQ must be equal and arithmetic
    if (r_type == l_type && r_base_type != TYPE_BOOL) {
      return TYPE_BOOL;
    }
    // TODO: log error. type mismatch
    return TYPE_UNKNOWN;
  default:
    break;
  }
  return TYPE_UNKNOWN;
}

symbol_type get_unary_expr_type(node *unary_node) {
  symbol_type type = unary_node->expression.unary.right->expression.expr_type;
  symbol_type base_type = get_base_type(type);

  switch (unary_node->expression.unary.op) {
  case OP_UMINUS:
    // Unary minus is an arithmetic operator, so only allow
    // base types of int and float
    if (base_type == TYPE_INT || base_type == TYPE_FLOAT) {
      // Return the original type (preserving scalar/vector)
      return type;
    }
    break;
  case OP_NOT:
    // Unary not is a logical operator, so only allow a boolean
    // base type
    if (base_type == TYPE_BOOL) {
      // Return the original type (preserving scalar/vector)
      return type;
    }
    break;
  default: break;
  }
  // Fall through to an unknown type
  return TYPE_UNKNOWN;
}

symbol_type get_function_return_type(node *func_node) {
  node *args = func_node->expression.function.arguments;
  switch (func_node->expression.function.func_id) {
  case FUNC_DP3:
    // Look at the first argument to determine the return type.
    if (args->argument.expression != NULL) {
      switch (args->argument.expression->expression.expr_type) {
      // If the first argument is TYPE_VEC[43], return TYPE_FLOAT
      case TYPE_VEC4: case TYPE_VEC3:
        return TYPE_FLOAT;
      // If the first argument is TYPE_IVEC[43], return TYPE_INT
      case TYPE_IVEC4: case TYPE_IVEC3:
        return TYPE_INT;
      // Otherwise fall through to TYPE_UNKNOWN
      default: break;
      }
    }
    break;
  case FUNC_RSQ:
    return TYPE_FLOAT;
    break;
  case FUNC_LIT:
    return TYPE_VEC4;
    break;
  }
  return TYPE_UNKNOWN;
}

symbol_type get_base_type(symbol_type type) {
  if (type & TYPE_VEC) {
    return TYPE_FLOAT;
  } else if (type & TYPE_IVEC) {
    return TYPE_INT;
  } else if (type & TYPE_BVEC) {
    return TYPE_BOOL;
  } else {
    return type;
  }
}

