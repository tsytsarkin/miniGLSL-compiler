#include <cstdlib>
#include "semantic.h"

int semantic_check( node *ast) {
  return 0; // failed checks
}

symbol_type get_binary_expr_type(node *binary_node) {
  node* right = binary_node->expression.binary.right;
  node* left = binary_node->expression.binary.left;
  // check if types match
  if (get_base_type(right->expression.expr_type) != get_base_type(left->expression.expr_type)){
    // TODO: log error (type mismatch)
    return TYPE_UNKNOWN;
  }
  symbol_type r_type = right->expression.expr_type;
  symbol_type l_type = left->expression.expr_type;
  bool r_is_vec = r_type & TYPE_ANY_VEC;
  bool l_is_vec = l_type & TYPE_ANY_VEC;
  binary_op op = binary_node->expression.binary.op;
  if(r_type == TYPE_UNKNOWN) //check for unknown input type
    return TYPE_UNKNOWN;
  switch (op) {
  case OP_AND:
  // fallthrough
  case OP_OR:
    if(r_type != l_type){
      // TODO: log error, type mismatch
      return TYPE_UNKNOWN;
    }
    if(r_type == TYPE_BOOL || r_type & TYPE_BVEC){
      return r_type;
    } else {
      // TODO: log error (not a boolean type)
      return TYPE_UNKNOWN;
    }
    break;
  case OP_PLUS:
  case OP_MINUS:
    if(r_type != l_type){
      // TODO: log error (type mismatch)
      return TYPE_UNKNOWN;
    }
    if(get_base_type(r_type) != TYPE_BOOL){
      return r_type;
    }
    break;
  case OP_DIV:
  case OP_XOR:
    if(r_type != l_type){
      // TODO: log error. Type mismatch
      return TYPE_UNKNOWN;
    }
    if(r_type == TYPE_INT || r_type == TYPE_FLOAT)
      return r_type;
    // TODO: log error. Unsupported type
    return TYPE_UNKNOWN;
    break;
  case OP_MUL:
    if(get_base_type(r_type) == TYPE_BOOL){
      // TODO: log error. Unsupported type
      return TYPE_UNKNOWN;
    }
    if(l_is_vec){
      if(r_is_vec){
        if(r_type == l_type){
          return get_base_type(r_type);
        } else {
          // log error. trying to multiply 2 vectors with different size
          return TYPE_UNKNOWN;
        }
      } else { // !r_is_vec
        return l_type;
      }
    } else { // !l_is_vec
      return r_type;
    }
    break;
  case OP_LT:
  case OP_LEQ:
  case OP_GT:
  case OP_GEQ:
    if(r_is_vec || l_is_vec){
      // TODO: log error. unsupported type
      return TYPE_UNKNOWN;
    }
    if(r_type == l_type && (r_type == TYPE_INT || r_type == TYPE_FLOAT))
        return r_type;
    // TODO: log error. type mismatch
    return TYPE_UNKNOWN;
    break;
  case OP_EQ:
  case OP_NEQ:
    if(r_type == l_type)
      return r_type;
    // TODO: log error. type mismatch
    return TYPE_UNKNOWN;
    break;
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

