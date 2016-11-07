%{
/***********************************************************************
 * Braden Watling
 * Nikita Tsytsarkin
 * g467-006
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
#define yTRACE_FORMAT(format, args...) { if (traceParser) fprintf(traceFile, format, args); }
#define yTRACE(x)                      { yTRACE_FORMAT("%s\n", x); }

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
%left           OR
%left           AND
%left           GEQ '>' LEQ '<' NEQ EQ
%left           '-' '+'
%left           '/' '*'
%right          '^'

// Unary operators
%left           '!' UMINUS

// Special
%left           CONSTRUCTOR FUNCTION VECTOR

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
  : scope { yTRACE("program"); }
  ;
scope
  : '{' declarations statements '}' { yTRACE("scope"); }
  ;
declarations
  : declarations declaration { yTRACE("declarations"); }
  | %empty { yTRACE("empty declarations"); }
  ;
statements
  : statements statement { yTRACE("statements"); }
  | %empty { yTRACE("empty statements"); }
  ;
declaration
  : type ID ';' { yTRACE_FORMAT("declaration (%s)\n", $2); }
  | type ID '=' expression ';' { yTRACE_FORMAT("assignment declaration (%s)\n", $2); }
  | CONST type ID '=' expression ';' { yTRACE_FORMAT("const assignment declaration (%s)\n", $3); }
  ;
statement
  : variable '=' expression ';' { yTRACE("assignment statement"); }
  | if_statement { yTRACE("if_statement statement"); }
  | WHILE '(' expression ')' statement { yTRACE("while statement"); }
  | scope { yTRACE("scope statement"); }
  | ';' { yTRACE("semicolon statement"); }
  ;
if_statement
  : IF '(' expression ')' statement %prec IF_THEN { yTRACE("if-then if_statement"); }
  | IF '(' expression ')' statement else_statement { yTRACE("if-then-else if_statement"); }
  ;
else_statement
  : ELSE statement { yTRACE("else_statement"); }
  ;
type
  : INT_T { yTRACE("INT_T type"); }
  | BOOL_T { yTRACE("BOOL_T type"); }
  | FLOAT_T { yTRACE("FLOAT_T type"); }
  | IVEC_T { yTRACE_FORMAT("IVEC%d_T type\n", $1); }
  | BVEC_T { yTRACE_FORMAT("BVEC%d_T type\n", $1); }
  | VEC_T { yTRACE_FORMAT("VEC%d_T type\n", $1); }
  ;
expression
  : constructor { yTRACE("constructor expression"); }
  | function { yTRACE("function expression"); }
  | INT_C { yTRACE_FORMAT("INT_C (%d) expression\n", $1); }
  | FLOAT_C { yTRACE_FORMAT("FLOAT_C (%f) expression\n", $1); }
  | variable { yTRACE("variable expression"); }
  | unary_op { yTRACE("unary_op expression"); }
  | binary_op { yTRACE("binary_op expression"); }
  | TRUE_C { yTRACE("TRUE_C expression"); }
  | FALSE_C { yTRACE("FALSE_C expression"); }
  | '(' expression ')' { yTRACE("brackets expression"); }
  ;
unary_op
  : '!' expression { yTRACE("! unary_op"); }
  | '-' expression %prec UMINUS { yTRACE("- unary_op"); }
  ;
binary_op
  : expression AND expression { yTRACE("&& binary_op"); }
  | expression OR expression { yTRACE("|| binary_op"); }
  | expression EQ expression { yTRACE("== binary_op"); }
  | expression NEQ expression  { yTRACE("!= binary_op"); }
  | expression '<' expression  { yTRACE("< binary_op"); }
  | expression LEQ expression { yTRACE("<= binary_op"); }
  | expression '>' expression { yTRACE("> binary_op"); }
  | expression GEQ expression { yTRACE(">= binary_op"); }
  | expression '+' expression { yTRACE("+ binary_op"); }
  | expression '-' expression { yTRACE("- binary_op"); }
  | expression '*' expression { yTRACE("* binary_op"); }
  | expression '/' expression { yTRACE("/ binary_op"); }
  | expression '^' expression { yTRACE("^ binary_op"); }
  ;
variable
  : ID { yTRACE_FORMAT("ID (%s) variable\n", $1); }
  | ID '[' INT_C ']' %prec VECTOR { yTRACE_FORMAT("vector variable (%s) [%d]\n", $1, $3); }
  ;
constructor
  : type '(' arguments ')' %prec CONSTRUCTOR { yTRACE("constructor"); }
  ;
function
  : function_name '(' arguments_opt ')' %prec FUNCTION { yTRACE("function"); }
  ;
function_name
  : FUNC { yTRACE("function_name"); }
  ;
arguments_opt
  : arguments { yTRACE("arguments"); }
  | %empty { yTRACE("empty arguments"); }
  ;
arguments
  : arguments ',' expression { /* No trace */ }
  | expression { /* No trace */ }
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

