#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string>
#include <vector>
#include <map>

#include "ast.h"

typedef struct {
  symbol_type type;
} symbol_info;

// The symbol tables for each scope, indexed by scope id
extern std::vector<std::map<std::string, symbol_info> > symbol_tables;

#endif

