#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"

std::vector<std::map<std::string, symbol_info> > symbol_tables;

void set_symbol_info(int scope_id, char *symbol_name, symbol_info sym_info) {
  symbol_tables[scope_id][symbol_name] = sym_info;
}

void init_symbol_table(std::map<std::string, symbol_info> &symbol_table){
  // Add pre defined variables to the symbol table
  // Create templates for different types
  symbol_info attribute;
  attribute.read_only = true;
  attribute.write_only = false;
  attribute.constant = false;
  attribute.type = TYPE_VEC4;

  symbol_info uniform;
  uniform.read_only = true;
  uniform.write_only = false;
  uniform.constant = true;
  uniform.type = TYPE_VEC4;

  symbol_info result;
  result.read_only = false;
  result.write_only = true;
  result.constant = false;
  result.type = TYPE_VEC4;

  symbol_table.insert(std::pair<std::string, symbol_info>("gl_FragColor", result));
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_FragCoord", result));
  result.type = TYPE_BOOL;   
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_FragDepth", result));

  symbol_table.insert(std::pair<std::string, symbol_info>("gl_TexCoord", attribute)); 
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_Color", attribute)); 
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_Secondary", attribute)); 
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_FogFragCoord", attribute)); 

  symbol_table.insert(std::pair<std::string, symbol_info>("gl_Light_Half", uniform));
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_Light_Ambient", uniform));
  symbol_table.insert(std::pair<std::string, symbol_info>("gl_Material_Shininess", uniform));
  symbol_table.insert(std::pair<std::string, symbol_info>("env1", uniform));
  symbol_table.insert(std::pair<std::string, symbol_info>("env2", uniform));
  symbol_table.insert(std::pair<std::string, symbol_info>("env3", uniform));
}

symbol_info get_symbol_info(const std::vector<unsigned int> &scope_id_stack, char *symbol_name) {
  std::vector<unsigned int>::const_reverse_iterator iter;

  // Traverse the scope id stack backwards
  for (iter = scope_id_stack.rbegin(); iter != scope_id_stack.rend(); iter++) {

    // Look at the symbol table for each scope
    const std::map<std::string, symbol_info> &symbol_table = symbol_tables[*iter];

    // Search for the symbol in the table
    std::map<std::string, symbol_info>::const_iterator symbol_iter = symbol_table.find(symbol_name);

    // If the symbol was found, return it
    if (symbol_iter != symbol_table.end()) {
      return symbol_iter->second;
    }
  }

  // If the symbol was not found, add a dummy symbol to the symbol table of the
  // current scope with TYPE_UNKNOWN and return it
  std::map<std::string, symbol_info> &symbol_table = symbol_tables[scope_id_stack.back()];

  symbol_info dummy_symbol_info;
  dummy_symbol_info.type = TYPE_UNKNOWN;
  dummy_symbol_info.read_only = false;
  dummy_symbol_info.write_only = false;
  dummy_symbol_info.constant = false;

  symbol_table[symbol_name] = dummy_symbol_info;

  return dummy_symbol_info;
}

