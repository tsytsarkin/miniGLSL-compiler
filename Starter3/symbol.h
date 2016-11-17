#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string>
#include <vector>
#include <map>
#include <vector>

#include "ast.h"

typedef struct {
  symbol_type type;
} symbol_info;

// The symbol tables for each scope, indexed by scope id
extern std::vector<std::map<std::string, symbol_info> > symbol_tables;

symbol_info get_symbol_info(const std::vector<int> &id_stack, char *symbol_name);

#endif

