
#include "semantic.h"

int semantic_check( node *ast) {
  return 0; // failed checks
}

symbol_type get_binary_expr_type(node *binary_node) {
  return TYPE_UNKNOWN;
}

symbol_type get_unary_expr_type(node *unary_node) {
  return TYPE_UNKNOWN;
}

symbol_type get_function_return_type(node *func_node) {
  switch (func_node->expression.function.func_id) {
  case FUNC_DP3:
    // TODO look at the arguments and figure out if we should return a float or an int
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

