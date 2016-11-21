%{
/**********************************************************************
 * Braden Watling
 * Nikita Tsytsarkin
 * g467-006
 *
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "ast.h"
#include "parser.tab.h"

#define YY_USER_INIT { yyin = inputFile; }
#define yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; }
#define yOUT(x)      { yTRACE(x); return x; }

/* forward declarations */
int ParseComment(void);
int ParseInt(void);
int ParseFloat(void);
int ParseIdent(void);
int yyline = 1, yycolumn = 1;

%}
%option noyywrap

%%

"/*"                          { yycolumn += yyleng; if(!ParseComment()) { yyterminate(); } }

[ \t]                         { yycolumn += yyleng; }
\r?\n                         { yyline++; yycolumn = 1; }
<<EOF>>                       { yyterminate(); }

[+\-*/\^]                     { yycolumn += yyleng; yOUT(yytext[0]); }
[()[\]{}]                     { yycolumn += yyleng; yOUT(yytext[0]); }
[<>=!]                        { yycolumn += yyleng; yOUT(yytext[0]); }
";"                           { yycolumn += yyleng; yOUT(yytext[0]); }
","                           { yycolumn += yyleng; yOUT(yytext[0]); }
"&&"                          { yycolumn += yyleng; yOUT(AND); }
"||"                          { yycolumn += yyleng; yOUT(OR); }
"!="                          { yycolumn += yyleng; yOUT(NEQ); }
"<="                          { yycolumn += yyleng; yOUT(LEQ); }
">="                          { yycolumn += yyleng; yOUT(GEQ); }
"=="                          { yycolumn += yyleng; yOUT(EQ); }

const                         { yycolumn += yyleng; yOUT(CONST); }
bool                          { yycolumn += yyleng; yOUT(BOOL_T); }
int                           { yycolumn += yyleng; yOUT(INT_T); }
float                         { yycolumn += yyleng; yOUT(FLOAT_T); }
vec(2|3|4)                    { yycolumn += yyleng; yylval.as_vec = yytext[3] - '0'; yOUT(VEC_T); }
ivec(2|3|4)                   { yycolumn += yyleng; yylval.as_vec = yytext[4] - '0'; yOUT(IVEC_T); }
bvec(2|3|4)                   { yycolumn += yyleng; yylval.as_vec = yytext[4] - '0'; yOUT(BVEC_T); }

if                            { yycolumn += yyleng; yOUT(IF); }
else                          { yycolumn += yyleng; yOUT(ELSE); }

dp3                           { yycolumn += yyleng; yylval.as_func = FUNC_DP3; yOUT(FUNC); }
rsq                           { yycolumn += yyleng; yylval.as_func = FUNC_RSQ; yOUT(FUNC); }
lit                           { yycolumn += yyleng; yylval.as_func = FUNC_LIT; yOUT(FUNC); }

true                          { yycolumn += yyleng; yOUT(TRUE_C); }
false                         { yycolumn += yyleng; yOUT(FALSE_C); }

0                             { yycolumn += yyleng; yylval.as_int = 0; yOUT(INT_C); }
[1-9][0-9]*                   { yycolumn += yyleng; if(ParseInt()) { yOUT(INT_C); } yyterminate(); }

(0|([1-9][0-9]*))\.[0-9]*     { yycolumn += yyleng; if(ParseFloat()) { yOUT(FLOAT_C); } yyterminate(); }
\.[0-9]+                      { yycolumn += yyleng; if(ParseFloat()) { yOUT(FLOAT_C); } yyterminate(); }

[A-Za-z_][A-Za-z0-9_]*        { yycolumn += yyleng; if(ParseIdent()) { yOUT(ID); } yyterminate(); }

0[0-9]+                       { yERROR("Octal numbers are not allowed.");  yyterminate(); }
[0-9]+[a-zA-Z_]+              { yERROR("Integers and identifiers/keywords must be separated by whitespace.");  yyterminate(); }
.                             { yERROR("Unknown token");  yyterminate(); }

%%

/* Constants used later */
enum {
  MAX_INT_LIT = (1 << 21) - 1,
  MIN_INT_LIT = -MAX_INT_LIT,
  MAX_IDENT_LEN = 32
};

/* Eat a C-style comment. */
int ParseComment(void) {
  int c1 = 0;
  int c2 = yyinput();
  int curline = yyline;
  for(;;) {
    if (c2 == EOF) {
      fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
      errorOccurred = TRUE;
      return 0;
    }
    if ('*' == c1 && '/' == c2) {
      break;
    }
    c1 = c2;
    c2 = yyinput();
    yycolumn++;
    if ('\n' == c1 && EOF != c2) {
      yyline++;
      yycolumn = 1;
    }
  }
  return 1;
}

/* Convert a string to an integer token. */
int ParseInt(void) {
  long num = strtol(yytext, NULL, 10);
  if(ERANGE == errno) {
    if(LONG_MAX == num || LONG_MIN == num) {
      yERROR("Integer literal is out of range (case 1).");
      return 0;
    }
  }

  if(MAX_INT_LIT < num || MIN_INT_LIT > num) {
    yERROR("Integer literal is out of range (case 2).");
    return 0;
  }

  yylval.as_int = (int) num;
  return 1;
}

/* Convert a string to a float token. */
int ParseFloat(void) {
  double num = strtod(yytext, NULL);
  
  if(ERANGE == errno) {
    if(HUGE_VAL == num || -HUGE_VAL == num) {
      yERROR("Floating point literal is out of range (case 1).");
      return 0;
    }
  }

  /* a NaN value; this is weird */
  if(num != num) {
    yERROR("Floating point literal is NaN.");
    return 0;
  }

  /* out of range; we consider overflow but not underflow */
  if(FLT_MAX < num || -FLT_MAX > num) {
    yERROR("Floating point literal is out of range (case 2).");
    return 0;
  }  

  yylval.as_float = (float) num;
  return 1;
}

/* Convert a string into an identifier token. */
int ParseIdent(void) {
  if(MAX_IDENT_LEN < yyleng) {
    yERROR("Identifier is too long.");
    return 0;
  }

  char *ident = (char *) calloc(yyleng + 1, sizeof(char));
  memcpy(ident, yytext, yyleng);

  yylval.as_str = ident;
  return 1; 
}

