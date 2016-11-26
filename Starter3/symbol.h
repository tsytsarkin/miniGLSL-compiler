#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string>
#include <vector>
#include <map>

#include "ast.h"

typedef struct {
  symbol_type type;
  bool read_only;
  bool write_only;
  bool constant;
  bool already_declared;
} symbol_info;

// The symbol tables for each scope, indexed by scope id
extern std::vector<std::map<std::string, symbol_info> > symbol_tables;

void set_symbol_info(int scope_id, char *symbol_name, symbol_info sym_info);
symbol_info* get_symbol_info(const std::vector<unsigned int> &scope_id_stack, char *symbol_name);
void init_symbol_table(std::map<std::string, symbol_info> &symbol_table);

#endif

