%{
/**********************************************************************
 *
 *  Braden Watling
 *  Nikita Tsytsarkin
 *  g467-006
 *
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"


#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) { fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); fflush(stdout); } }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS                      [ \t]
NEWLINE                 \n
DIGIT                   [0-9]
IDENT_CHAR              [a-zA-Z_0-9]

%%
{WS}                    { }

{NEWLINE}               { yyline++; }

"/*"                    {
                          int c1 = 0;
                          int c2 = yyinput();
                          int curline = yyline;
                          for(;;) {
                            if (c2 == EOF) {
                              fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
                              errorOccurred = TRUE;
                              yyterminate();
                            }
                            if (c1 == '*' && c2 == '/')
                              break;
                            c1 = c2;
                            c2 = yyinput();
                            if (c1 == '\n' && c2 != EOF)
                              yyline++;
                          }
                        }

{DIGIT}+"."{DIGIT}*     {
                          float floatval = strtof(yytext, NULL);
                          if (errno == ERANGE) {
                            yERROR("Float out of bounds");
                          }
                          yylval.floatval = floatval;
                          yTRACE(TOK_FLOAT);
                          return TOK_FLOAT;
                        }

{DIGIT}+                {
                          // Assume that integers cannot start with 0
                          if (strlen(yytext) > 1 && yytext[0] == '0') {
                            yERROR("Integer cannot start with a 0");
                          }

                          int intval = atoi(yytext);
                          if (intval < 0 || intval > MAX_INTEGER) {
                            yERROR("Integer out of bounds");
                          }
                          yylval.intval = intval;
                          yTRACE(TOK_INTEGER);
                          return TOK_INTEGER;
                        }

"if"                    {
                          yTRACE(TOK_IF);
                          return TOK_INTEGER;
                        }

"else"                  {
                          yTRACE(TOK_ELSE);
                          return TOK_ELSE;
                        }

"while"                 {
                          yTRACE(TOK_WHILE);
                          return TOK_WHILE;
                        }

"const"                 {
                          yTRACE(TOK_CONST);
                          return TOK_CONST;
                        }

"int"                   {
                          yTRACE(TOK_INT_TYPENAME);
                          return TOK_INT_TYPENAME;
                        }

"float"                 {
                          yTRACE(TOK_FLOAT_TYPENAME);
                          return TOK_FLOAT_TYPENAME;
                        }

"vec2"                  {
                          yTRACE(TOK_VEC2_TYPENAME);
                          return TOK_VEC2_TYPENAME;
                        }

"vec3"                  {
                          yTRACE(TOK_VEC3_TYPENAME);
                          return TOK_VEC3_TYPENAME;
                        }

"vec4"                  {
                          yTRACE(TOK_VEC3_TYPENAME);
                          return TOK_VEC3_TYPENAME;
                        }

"ivec2"                 {
                          yTRACE(TOK_IVEC2_TYPENAME);
                          return TOK_IVEC2_TYPENAME;
                        }

"ivec3"                 {
                          yTRACE(TOK_IVEC3_TYPENAME);
                          return TOK_IVEC3_TYPENAME;
                        }

"ivec4"                 {
                          yTRACE(TOK_IVEC3_TYPENAME);
                          return TOK_IVEC3_TYPENAME;
                        }

"bvec2"                 {
                          yTRACE(TOK_BVEC2_TYPENAME);
                          return TOK_BVEC2_TYPENAME;
                        }

"bvec3"                 {
                          yTRACE(TOK_BVEC3_TYPENAME);
                          return TOK_BVEC3_TYPENAME;
                        }

"bvec4"                 {
                          yTRACE(TOK_BVEC3_TYPENAME);
                          return TOK_BVEC3_TYPENAME;
                        }

"true"                  {
                          yTRACE(TOK_TRUE);
                          return TOK_TRUE;
                        }

"false"                 {
                          yTRACE(TOK_FALSE);
                          return TOK_FALSE;
                        }

"dp3"                   {
                          yTRACE(TOK_FUNC_DP3);
                          return TOK_FUNC_DP3;
                        }

"lit"                   {
                          yTRACE(TOK_FUNC_LIT);
                          return TOK_FUNC_LIT;
                        }

"rsq"                   {
                          yTRACE(TOK_FUNC_RSQ);
                          return TOK_FUNC_RSQ;
                        }

{IDENT_CHAR}+           {
                          if (yytext[0] >= '0' && yytext[0] <= '9') {
                            yERROR("Identifier cannot start with a number");
                          }
                          if (strlen(yytext) > MAX_IDENTIFIER) {
                            yERROR("Identifier exceeds maximum length of 32 characters");
                          }
                          yylval.strval = yytext;
                          yTRACE(TOK_IDENTIFIER);
                          return TOK_IDENTIFIER;
                        }

\"([^\\\"]|\\.)*\"?     {
// Match anything within quotes. If the character is a '\' then it must be used as an
// escape character. We also cannot match a '"' within a string, unless it is escaped.
// The string should end in a '"', but if it doesn't we still want to match it and
// execute this code so that we can throw the correct error (unmatched '"').
                          unsigned int len = strlen(yytext);
                          if (yytext[len - 1] != '"') {
                            yERROR("Unmatched '\"'");
                          }
                          // Compensate for yytext containing 2 '"' characters
                          if (len > MAX_TEXT + 2) {
                            yERROR("String length exceeds max length");
                          }
                          unsigned int i;
                          for (i = 0; i < len; i++) {
                            if (yytext[i] == '\n') {
                              yERROR("String cannot span multiple lines");
                            }
                          }
                          yTRACE(TOK_STRING);
                          return TOK_STRING;
                        }

"-"                     {
                          yTRACE(TOK_MINUS);
                          return TOK_MINUS;
                        }

"+"                     {
                          yTRACE(TOK_PLUS);
                          return TOK_PLUS;
                        }

"="                     {
                          yTRACE(TOK_EQUAL);
                          return TOK_EQUAL;
                        }

"=="                    {
                          yTRACE(TOK_EQUAL_EQUAL);
                          return TOK_EQUAL_EQUAL;
                        }

"<="                    { 
                          yTRACE(TOK_LESS_OR_EQUAL);
                          return TOK_LESS_OR_EQUAL;
                        }

">="                    {
                          yTRACE(TOK_GREATER_OR_EQUAL);
                          return TOK_GREATER_OR_EQUAL;
                        }

"*"                     {
                          yTRACE(TOK_STAR);
                          return TOK_STAR;
                        }

"/"                     {
                          yTRACE(TOK_SLASH);
                          return TOK_SLASH;
                        }

"^"                     {
                          yTRACE(TOK_CARET);
                          return TOK_CARET;
                        }

"!"                     {
                          yTRACE(TOK_NOT);
                          return TOK_NOT;
                        }

"!="                    {
                          yTRACE(TOK_NOT_EQUAL);
                          return TOK_NOT_EQUAL;
                        }

"&&"                    {
                          yTRACE(TOK_AND);
                          return TOK_AND;
                        }

"||"                    {
                          yTRACE(TOK_OR);
                          return TOK_OR;
                        }

"<"                     {    
                          yTRACE(TOK_LESS);
                          return TOK_LESS;
                        }

">"                     {
                          yTRACE(TOK_GREATER);
                          return TOK_GREATER;
                        }

"("                     {
                          yTRACE(TOK_BRACKET_OPEN);
                          return TOK_BRACKET_OPEN;
                        }

")"                     {
                          yTRACE(TOK_BRACKET_CLOSE);
                          return TOK_BRACKET_CLOSE;
                        }

"["                     {
                          yTRACE(TOK_SQUARE_BRACKET_OPEN);
                          return TOK_SQUARE_BRACKET_OPEN;
                        }

"]"                     {
                          yTRACE(TOK_SQUARE_BRACKET_CLOSE);
                          return TOK_SQUARE_BRACKET_CLOSE;
                        }

":"                     {
                          yTRACE(TOK_COLON);
                          return TOK_COLON;
                        }

";"                     {
                          yTRACE(TOK_SEMICOLON);
                          return TOK_SEMICOLON;
                        }

","                     {
                          yTRACE(TOK_COMMA);
                          return TOK_COMMA;
                        }

"{"                     {
                          yTRACE(TOK_CURLY_BRACKET_OPEN);
                          return TOK_CURLY_BRACKET_OPEN;
                        }

"}"                     {
                          yTRACE(TOK_CURLY_BRACKET_CLOSE);
                          return TOK_CURLY_BRACKET_CLOSE;
                        }

.                       { yERROR("Unknown token"); }

%%
