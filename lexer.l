%{
#include "parser.tab.h"

#include <stdio.h>
#include <stdlib.h>



extern int yywrap();

%}

%option yylineno
%option yywrap
%option debug
%option nounput noinput


%%

\n { ++yylineno; }
[ \t]+ { /* ignore whitespace */ }

"int"              { return INT; }
"float"            { return FLOAT; }
"string"           { return STRING; }
"void"             { return VOID; }

"if" { return IF; }
"else" { return ELSE; }
"while" { return WHILE; }
"return" { return RETURN; }

[0-9]+             { yylval.intValue = atoi(yytext); return NUMBER; }
[0-9]+"."[0-9]*    { yylval.floatValue = atof(yytext); return FLOAT; }
\"[^"]*\"          { yylval.strValue = strdup(yytext); return STRING; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.identifier = strdup(yytext); return IDENTIFIER; }

"[" { return LBRACKET; }
"]" { return RBRACKET; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"(" { return LPAREN; }
")" { return RPAREN; }
"," { return COMMA; }
";" { return SEMICOLON; }
"+" { return PLUS; }
"=" { return ASSIGN; }

. {
    fprintf(stderr, "Unexpected character '%s' at line %d\n", yytext, yylineno);
    exit(1);
}

%%

int yywrap() {return (1);}