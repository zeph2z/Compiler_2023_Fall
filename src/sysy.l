%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>
#include "sysy.tab.hpp"

using namespace std;

%}

WhiteSpace    [ \t\n\r]*
LineComment   "//".*|"/\*".*"\*/"

Identifier    [a-zA-Z_][a-zA-Z0-9_]*

Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%%

{WhiteSpace}    { }
{LineComment}   { }

"int"           { return INT; }
"return"        { return RETURN; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

"+"             { yylval.char_val = '+'; return PLUS; }
"-"             { yylval.char_val = '-'; return MINUS; }
"!"             { yylval.char_val = '!'; return NOT; }
"*"             { yylval.char_val = '*'; return TIMES; }
"/"             { yylval.char_val = '/'; return DIVIDE; }
"%"             { yylval.char_val = '%'; return MOD; }

.               { return yytext[0]; }

%%
