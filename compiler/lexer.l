%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int yycolumn = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

%option noyywrap
%option yylineno

%%

[ \t]+          { /* skip whitespace */ }
\n              { yycolumn = 1; }
"//".*          { /* skip single-line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* skip multi-line comments */ }

    /* Keywords */
"loop"          { return LOOP; }
"for"           { return FOR; }
"in"            { return IN; }
"if"            { return IF; }
"else"          { return ELSE; }
"when"          { return WHEN; }
"unless"        { return UNLESS; }
"and"           { return AND; }
"or"            { return OR; }
"not"           { return NOT; }
"true"          { return TRUE; }
"false"         { return FALSE; }
"@use"          { return USE; }

    /* Word-based comparison operators */
"lt"            { return LT; }
"gt"            { return GT; }
"le"            { return LE; }
"ge"            { return GE; }

    /* Multi-char operators (must come before single-char) */
":="            { return ASSIGN_DECL; }
"<<"            { return RETURN; }
">>"            { return BREAK; }
"=>"            { return ARROW; }
".."            { return RANGE; }
"->"            { return MEMBER_ACCESS; }
"=="            { return EQ; }
"!="            { return NE; }
"{"             { return OBJ_OPEN; }
"}"             { return OBJ_CLOSE; }

    /* Single-char tokens */
"."             { return DOT; }
","             { return COMMA; }
":"             { return COLON; }
"="             { return ASSIGN; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return STAR; }
"/"             { return SLASH; }
"%"             { return PERCENT; }
"("             { return LPAREN; }
")"             { return RPAREN; }
">"             { return LBRACE; }
"<"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
"|"             { return PIPE; }
"#"             { return HASH; }
"\\"            { return BACKSLASH; }
"_"             { return UNDERSCORE; }

    /* Literals */
[0-9]+\.[0-9]+f { 
                    yylval.fval = atof(yytext);
                    return FLOAT_LITERAL;
                }

[0-9]+          { 
                    yylval.ival = atoi(yytext); 
                    return INT_LITERAL; 
                }

\"[^\"]*\"      {
                    yylval.sval = strndup(yytext + 1, yyleng - 2);
                    return STRING_LITERAL;
                }

    /* Identifiers */
[a-zA-Z_][a-zA-Z0-9_]* { 
                    yylval.sval = strdup(yytext); 
                    return IDENTIFIER; 
                }

.               { 
                    fprintf(stderr, "Unexpected character: %s at line %d\n", yytext, yylineno); 
                }

%%
