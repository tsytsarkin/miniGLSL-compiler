#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"

std::vector<std::map<std::string, symbol_info> > symbol_tables;

const symbol_info *get_symbol_info(const std::vector<int> &scope_id_stack, char *symbol_name) {
  std::vector<int>::const_reverse_iterator iter;

  // Traverse the scope id stack backwards
  for (iter = scope_id_stack.rend(); iter != scope_id_stack.rbegin(); iter++) {

    // Look at the symbol table for each scope
    const std::map<std::string, symbol_info> &symbol_table = symbol_tables[*iter];

    // Search for the symbol in the table
    std::map<std::string, symbol_info>::const_iterator symbol_iter = symbol_table.find(symbol_name);

    // If the symbol was found, return it
    if (symbol_iter != symbol_table.end()) {
      return &symbol_iter->second;
    }
  }

  // Otherwise, return NULL
  return NULL;
}
