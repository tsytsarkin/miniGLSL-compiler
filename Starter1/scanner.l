%{
/**********************************************************************
 *
 * **YOUR GROUP INFO SHOULD GO HERE**
 *
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"


#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS                      [ \t]
NEWLINE                 \n
DIGIT                   [0-9]
IDENT_CHAR              [a-zA-Z_]

%%
{WS}                    { }

{NEWLINE}               { yyline++; }

"/*"                    {
                          int c1      = 0;
                          int c2      = yyinput();
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
                        }

{DIGIT}+                {
                          int intval = atoi(yytext);
                          if (intval < 0 || intval > MAX_INTEGER) {
                            yERROR("Integer out of bounds");
                          }
                          yylval.intval = intval;
                          yTRACE(TOK_INTEGER);
                        }

[a-zA-Z_0-9]+           {
                          if (yytext[0] >= '0' && yytext[0] <= '9') {
                            yERROR("Identifier cannot start with a number");
                          }
                          if (strlen(yytext) > MAX_IDENTIFIER) {
                            yERROR("Identifier exceeds maximum length of 32 characters");
                          }
                          yylval.strval = yytext;
                          yTRACE(TOK_IDENTIFIER);
                        }

"-"                     {
                          yTRACE(TOK_MINUS);
                        }

"+"                     {
                          yTRACE(TOK_PLUS);
                        }

"="                     {
                          yTRACE(TOK_EQUAL);
                        }

"=="                    {
                          yTRACE(TOK_EQUAL_EQUAL);
                        }

"<="                    { 
                          yTRACE(TOK_LESS_OR_EQUAL);
                        }

">="                    {
                          yTRACE(TOK_GREATER_OR_EQUAL);
                        }

"*"                     {
                          yTRACE(TOK_STAR);
                        }

"/"                     {
                          yTRACE(TOK_SLASH);
                        }

"^"                     {
                          yTRACE(TOK_CARET);
                        }

"!"                     {
                          yTRACE(TOK_NOT);
                        }

"!="                    {
                          yTRACE(TOK_NOT_EQUAL);
                        }

"&&"                    {
                          yTRACE(TOK_AND);
                        }

"||"                    {
                          yTRACE(TOK_OR);
                        }

"<"                     {    
                          yTRACE(TOK_LESS);
                        }

">"                     {
                          yTRACE(TOK_GREATER);
                        }

"("                     {
                          yTRACE(TOK_BRACKET_OPEN);
                        }

")"                     {
                          yTRACE(TOK_BRACKET_CLOSE);
                        }

"["                     {
                          yTRACE(TOK_SQUARE_BRACKET_OPEN);
                        }

"]"                     {
                          yTRACE(TOK_SQUARE_BRACKET_CLOSE);
                        }

":"                     {
                          yTRACE(TOK_COLON);
                        }

";"                     {
                          yTRACE(TOK_SEMICOLON);
                        }

","                     {
                          yTRACE(TOK_COMMA);
                        }

"{"                     {
                          yTRACE(TOK_SQUIGGLY_BRACKET_OPEN);
                        }

"}"                     {
                          yTRACE(TOK_SQUIGGLY_BRACKET_CLOSE);
                        }

.                       { yERROR("Unknown token"); }

%%
