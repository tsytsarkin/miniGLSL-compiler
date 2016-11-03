%{
/***********************************************************************
 * --YOUR GROUP INFO SHOULD GO HERE--
 * 
 *   Interface to the parser module for CSC467 course project.
 * 
 *   Phase 2: Implement context free grammar for source language, and
 *            parse tracing functionality.
 *   Phase 3: Construct the AST for the source language program.
 ***********************************************************************/

/***********************************************************************
 *  C Definitions and external declarations for this module.
 *
 *  Phase 3: Include ast.h if needed, and declarations for other global or
 *           external vars, functions etc. as needed.
 ***********************************************************************/

#include <string.h>
#include "common.h"
//#include "ast.h"
//#include "symbol.h"
//#include "semantic.h"
#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(const char* s);    /* what to do in case of error            */
int yylex();                    /* procedure for calling lexical analyzer */
extern int yyline;              /* variable holding current line number   */

enum {
  DP3 = 0,
  LIT = 1,
  RSQ = 2
};

%}

/***********************************************************************
 *  Yacc/Bison declarations.
 *  Phase 2:
 *    1. Add precedence declarations for operators (after %start declaration)
 *    2. If necessary, add %type declarations for some nonterminals
 *  Phase 3:
 *    1. Add fields to the union below to facilitate the construction of the
 *       AST (the two existing fields allow the lexical analyzer to pass back
 *       semantic info, so they shouldn't be touched).
 *    2. Add <type> modifiers to appropriate %token declarations (using the
 *       fields of the union) so that semantic information can by passed back
 *       by the scanner.
 *    3. Make the %type declarations for the language non-terminals, utilizing
 *       the fields of the union as well.
 ***********************************************************************/

%{
#define YYDEBUG 1
%}

// defines the yyval union
%union {
  int as_int;
  int as_vec;
  float as_float;
  char *as_str;
  int as_func;
}

%token          FLOAT_T
%token          INT_T
%token          BOOL_T
%token          CONST
%token          FALSE_C TRUE_C
%token          FUNC
%token          IF WHILE ELSE
%token          AND OR NEQ EQ LEQ GEQ

// links specific values of tokens to yyval
%token <as_vec>   VEC_T
%token <as_vec>   BVEC_T
%token <as_vec>   IVEC_T
%token <as_float> FLOAT_C
%token <as_int>   INT_C
%token <as_str>   ID

%start    program

// Lowest precedence

// Binary operators
%left           OR AND GEQ '>' LEQ '<' NEQ EQ
%left           '-' '+'
%left           '/' '*'
%right          '^'

// Unary operators
%left           '!' UMINUS
%left           CONSTRUCTOR FUNCTION VECTOR

// Give the reduction of a binary expression higher
// precendence than the shifting of unary and binary operators
%precedence     BINARY_EXPRESSION

// Give the reduction of a unary expression higher
// precedence than the reduction of a binary expression
// and the shifting of unary and binary operators
%precedence     UNARY_EXPRESSION

// Give the shifting of else a higher precedence than
// the reduction of if-then
%precedence     IF_THEN
%precedence     ELSE

// Highest precedence

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 ***********************************************************************/

program
  : scope
  ;
scope
  : '{' declarations statements '}'
  ;
declarations
  : declarations declaration
  | %empty
  ;
statements
  : statements statement
  | %empty
  ;
declaration
  : type ID ';'
  | type ID '=' expression ';'
  | CONST type ID '=' expression ';'
  ;
statement
  : variable '=' expression ';'
  | if_statement
  | WHILE '(' expression ')' statement
  | scope
  | ';'
  ;
if_statement
  : IF '(' expression ')' statement %prec IF_THEN
  | IF '(' expression ')' statement else_statement
  ;
else_statement
  : ELSE statement
  ;
type
  : INT_T
  | BOOL_T
  | FLOAT_T
  | IVEC_T
  | BVEC_T
  | VEC_T
  ;
expression
  : constructor
  | function
  | INT_C
  | FLOAT_C
  | variable
  | unary_op expression %prec UNARY_EXPRESSION
  | expression binary_op expression %prec BINARY_EXPRESSION
  | TRUE_C
  | FALSE_C
  | '(' expression ')'
  ;
variable
  : ID
  | ID '[' INT_C ']' %prec VECTOR
  ;
unary_op
  : '!'
  | '-' %prec UMINUS
  ;
binary_op
  : AND
  | OR
  | EQ
  | NEQ
  | '<'
  | LEQ
  | '>'
  | GEQ
  | '+'
  | '-'
  | '*'
  | '/'
  | '^'
  ;
constructor
  : type '(' arguments ')' %prec CONSTRUCTOR
  ;
function
  : function_name '(' arguments_opt ')' %prec FUNCTION
  ;
function_name
  : FUNC
  ;
arguments_opt
  : arguments
  | %empty
  ;
arguments
  : arguments ',' expression
  | expression
  ;

%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(const char* s) {
  if(errorOccurred) {
    return;    /* Error has already been reported by scanner */
  } else {
    errorOccurred = 1;
  }

  fprintf(errorFile, "\nPARSER ERROR, LINE %d", yyline);

  if(strcmp(s, "parse error")) {
    if(strncmp(s, "parse error, ", 13)) {
      fprintf(errorFile, ": %s\n", s);
    } else {
      fprintf(errorFile, ": %s\n", s+13);
    }
  } else {
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
  }
}

