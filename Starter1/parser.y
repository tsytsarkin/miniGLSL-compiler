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

void yyerror(char const* s);    /* what to do in case of error            */
int yylex();                    /* procedure for calling lexical analyzer */
extern int yyline;              /* variable holding current line number   */

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


// Access these values in flex using yylval
%union {
  int intval;
  float floatval;
  char *strval;
}
// The tokens
%token           TOK_FLOAT
%token           TOK_INTEGER
%token           TOK_IDENTIFIER
%token           TOK_STRING
%token           TOK_MINUS
%token           TOK_PLUS
%token           TOK_EQUAL
%token           TOK_EQUAL_EQUAL
%token           TOK_LESS_OR_EQUAL
%token           TOK_GREATER_OR_EQUAL
%token           TOK_STAR
%token           TOK_SLASH
%token           TOK_CARET
%token           TOK_NOT
%token           TOK_NOT_EQUAL
%token           TOK_AND
%token           TOK_OR
%token           TOK_LESS
%token           TOK_GREATER
%token           TOK_BRACKET_OPEN
%token           TOK_BRACKET_CLOSE
%token           TOK_SQUARE_BRACKET_OPEN
%token           TOK_SQUARE_BRACKET_CLOSE
%token           TOK_COLON
%token           TOK_SEMICOLON
%token           TOK_COMMA
%token           TOK_CURLY_BRACKET_OPEN
%token           TOK_CURLY_BRACKET_CLOSE
%token           TOK_IF
%token           TOK_ELSE
%token           TOK_WHILE
%token           TOK_CONST
%token           TOK_INT_TYPENAME
%token           TOK_BOOL_TYPENAME
%token           TOK_FLOAT_TYPENAME
%token           TOK_VEC2_TYPENAME
%token           TOK_VEC3_TYPENAME
%token           TOK_VEC4_TYPENAME
%token           TOK_IVEC2_TYPENAME
%token           TOK_IVEC3_TYPENAME
%token           TOK_IVEC4_TYPENAME
%token           TOK_BVEC2_TYPENAME
%token           TOK_BVEC3_TYPENAME
%token           TOK_BVEC4_TYPENAME


%start    program

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 *  Phase 3:
 *    1. Add code to rules for construction of AST.
 ***********************************************************************/
program
  :   tokens       
  ;
tokens
  :  tokens token  
  |      
  ;
token
  :     TOK_FLOAT
  |     TOK_INTEGER
  |     TOK_MINUS
  |     TOK_PLUS
  ;


%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(char const* s) {
  if (errorOccurred)
    return;    /* Error has already been reported by scanner */
  else
    errorOccurred = 1;
        
  fprintf(errorFile, "\nPARSER ERROR, LINE %d",yyline);
  if (strcmp(s, "parse error")) {
    if (strncmp(s, "parse error, ", 13))
      fprintf(errorFile, ": %s\n", s);
    else
      fprintf(errorFile, ": %s\n", s+13);
  } else
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
}

