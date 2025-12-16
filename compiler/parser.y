%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

void yyerror(const char *s);
%}

%locations

%union {
    int ival;
    double fval;
    char *sval;
    struct ASTNode *node;
    struct ASTList *list;
}

%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token <sval> IDENTIFIER STRING_LITERAL

%token LOOP FOR IN IF ELSE WHEN UNLESS
%token AND OR NOT TRUE FALSE
%token ASSIGN_DECL RETURN BREAK ARROW RANGE MEMBER_ACCESS
%token EQ NE LE GE LT GT
%token DOT COMMA COLON ASSIGN
%token PLUS MINUS STAR SLASH PERCENT
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token PIPE HASH BACKSLASH UNDERSCORE OBJ_OPEN OBJ_CLOSE
%token USE

%type <node> program top_level func_def use_stmt
%type <node> block statement assign_stmt
%type <node> var_decl loop_stmt for_stmt
%type <node> expr cond_expr or_expr and_expr
%type <node> eq_expr rel_expr add_expr mul_expr unary_expr
%type <node> postfix_expr primary_expr wand_call wand_arg
%type <node> lambda pattern_match match_arm object_field
%type <list> top_level_list statement_list wand_args param_list param_list_opt
%type <list> expr_list expr_list_opt match_arms array_items object_fields object_fields_opt

%left PIPE
%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH PERCENT
%right NOT
%right UMINUS

%start program

%%

program
    : top_level_list { ast_root = ast_new_program($1); }
    ;

top_level_list
    : top_level { $$ = ast_list_new(); ast_list_append($$, $1); }
    | top_level_list top_level { ast_list_append($1, $2); $$ = $1; }
    ;

top_level
    : func_def { $$ = $1; }
    | var_decl { $$ = $1; }
    | use_stmt { $$ = $1; }
    ;

use_stmt
    : USE STRING_LITERAL DOT { $$ = ast_new_use($2); }
    ;

/* Function definition: #name(args) { } or #name(args) => expr. */
func_def
    : HASH IDENTIFIER LPAREN param_list_opt RPAREN block {
        $$ = ast_new_function($2, $4, $6);
    }
    | HASH IDENTIFIER LPAREN param_list_opt RPAREN ARROW expr DOT {
        $$ = ast_new_function($2, $4, ast_new_return($7));
    }
    ;

param_list_opt
    : /* empty */ { $$ = ast_list_new(); }
    | param_list { $$ = $1; }
    ;

param_list
    : IDENTIFIER { $$ = ast_list_new(); ast_list_append($$, ast_new_param($1)); }
    | param_list COMMA IDENTIFIER { ast_list_append($1, ast_new_param($3)); $$ = $1; }
    ;

block
    : LBRACE statement_list RBRACE { $$ = ast_new_block($2); }
    ;

statement_list
    : /* empty */ { $$ = ast_list_new(); }
    | statement_list statement { ast_list_append($1, $2); $$ = $1; }
    ;

statement
    : var_decl { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | loop_stmt { $$ = $1; }
    | for_stmt { $$ = $1; }
    | RETURN expr DOT { $$ = ast_new_return($2); }
    | RETURN expr WHEN expr DOT { 
        $$ = ast_new_when_stmt(ast_new_return($2), $4, 0); 
    }
    | RETURN DOT { $$ = ast_new_return(NULL); }
    | BREAK DOT { $$ = ast_new_break(NULL); }
    | BREAK WHEN expr DOT { $$ = ast_new_break($3); }
    | expr DOT { $$ = ast_new_expr_stmt($1); }
    | expr WHEN expr DOT { $$ = ast_new_when_stmt($1, $3, 0); }
    | expr UNLESS expr DOT { $$ = ast_new_when_stmt($1, $3, 1); }
    | block { $$ = $1; }
    ;

var_decl
    : IDENTIFIER ASSIGN_DECL expr DOT { $$ = ast_new_var_decl($1, $3); }
    ;

assign_stmt
    : IDENTIFIER ASSIGN expr DOT { $$ = ast_new_assign(ast_new_identifier($1), $3); }
    | IDENTIFIER ASSIGN expr WHEN expr DOT { 
        $$ = ast_new_when_stmt(ast_new_assign(ast_new_identifier($1), $3), $5, 0); 
    }
    | IDENTIFIER ASSIGN expr UNLESS expr DOT { 
        $$ = ast_new_when_stmt(ast_new_assign(ast_new_identifier($1), $3), $5, 1); 
    }
    | postfix_expr ASSIGN expr DOT { $$ = ast_new_assign($1, $3); }
    | postfix_expr ASSIGN expr WHEN expr DOT { 
        $$ = ast_new_when_stmt(ast_new_assign($1, $3), $5, 0); 
    }
    | postfix_expr ASSIGN expr UNLESS expr DOT { 
        $$ = ast_new_when_stmt(ast_new_assign($1, $3), $5, 1); 
    }
    ;

loop_stmt
    : LOOP block { $$ = ast_new_loop(NULL, $2); }
    | LOOP WHEN expr block { $$ = ast_new_loop($3, $4); }
    ;

for_stmt
    : FOR IDENTIFIER IN expr block {
        $$ = ast_new_for($2, $4, $5);
    }
    ;

expr
    : cond_expr { $$ = $1; }
    | expr PIPE cond_expr { $$ = ast_new_pipe($1, $3); }
    | expr PIPE pattern_match { $$ = ast_new_pipe($1, $3); }
    ;

cond_expr
    : or_expr { $$ = $1; }
    | or_expr IF or_expr ELSE cond_expr {
        $$ = ast_new_ternary($3, $1, $5);
    }
    ;

or_expr
    : and_expr { $$ = $1; }
    | or_expr OR and_expr { $$ = ast_new_binary(OP_OR, $1, $3); }
    ;

and_expr
    : eq_expr { $$ = $1; }
    | and_expr AND eq_expr { $$ = ast_new_binary(OP_AND, $1, $3); }
    ;

eq_expr
    : rel_expr { $$ = $1; }
    | eq_expr EQ rel_expr { $$ = ast_new_binary(OP_EQ, $1, $3); }
    | eq_expr NE rel_expr { $$ = ast_new_binary(OP_NE, $1, $3); }
    ;

rel_expr
    : add_expr { $$ = $1; }
    | rel_expr LT add_expr { $$ = ast_new_binary(OP_LT, $1, $3); }
    | rel_expr GT add_expr { $$ = ast_new_binary(OP_GT, $1, $3); }
    | rel_expr LE add_expr { $$ = ast_new_binary(OP_LE, $1, $3); }
    | rel_expr GE add_expr { $$ = ast_new_binary(OP_GE, $1, $3); }
    ;

add_expr
    : mul_expr { $$ = $1; }
    | add_expr PLUS mul_expr { $$ = ast_new_binary(OP_ADD, $1, $3); }
    | add_expr MINUS mul_expr { $$ = ast_new_binary(OP_SUB, $1, $3); }
    ;

mul_expr
    : unary_expr { $$ = $1; }
    | mul_expr STAR unary_expr { $$ = ast_new_binary(OP_MUL, $1, $3); }
    | mul_expr SLASH unary_expr { $$ = ast_new_binary(OP_DIV, $1, $3); }
    | mul_expr PERCENT unary_expr { $$ = ast_new_binary(OP_MOD, $1, $3); }
    ;

unary_expr
    : postfix_expr { $$ = $1; }
    | MINUS unary_expr %prec UMINUS { $$ = ast_new_unary(OP_NEG, $2); }
    | NOT unary_expr { $$ = ast_new_unary(OP_NOT, $2); }
    ;

postfix_expr
    : primary_expr { $$ = $1; }
    | postfix_expr LBRACKET expr RBRACKET { $$ = ast_new_index($1, $3); }
    | postfix_expr MEMBER_ACCESS IDENTIFIER { $$ = ast_new_member($1, $3); }
    ;

primary_expr
    : INT_LITERAL { $$ = ast_new_int_literal($1); }
    | FLOAT_LITERAL { $$ = ast_new_float_literal($1); }
    | STRING_LITERAL { $$ = ast_new_string_literal($1); }
    | TRUE { $$ = ast_new_bool_literal(1); }
    | FALSE { $$ = ast_new_bool_literal(0); }
    | IDENTIFIER { $$ = ast_new_identifier($1); }
    | UNDERSCORE { $$ = ast_new_implicit(); }
    | LPAREN expr RPAREN { $$ = $2; }
    | wand_call { $$ = $1; }
    | lambda { $$ = $1; }
    | LBRACKET expr_list_opt RBRACKET { $$ = ast_new_array($2); }
    | OBJ_OPEN object_fields_opt OBJ_CLOSE { $$ = ast_new_object($2); }
    | INT_LITERAL RANGE INT_LITERAL { 
        $$ = ast_new_range(ast_new_int_literal($1), ast_new_int_literal($3)); 
    }
    | INT_LITERAL RANGE IDENTIFIER { 
        $$ = ast_new_range(ast_new_int_literal($1), ast_new_identifier($3)); 
    }
    | INT_LITERAL RANGE LPAREN expr RPAREN { 
        $$ = ast_new_range(ast_new_int_literal($1), $4); 
    }
    | IDENTIFIER RANGE INT_LITERAL { 
        $$ = ast_new_range(ast_new_identifier($1), ast_new_int_literal($3)); 
    }
    | IDENTIFIER RANGE IDENTIFIER { 
        $$ = ast_new_range(ast_new_identifier($1), ast_new_identifier($3)); 
    }
    | IDENTIFIER RANGE LPAREN expr RPAREN { 
        $$ = ast_new_range(ast_new_identifier($1), $4); 
    }
    | LPAREN expr RPAREN RANGE INT_LITERAL { 
        $$ = ast_new_range($2, ast_new_int_literal($5)); 
    }
    | LPAREN expr RPAREN RANGE IDENTIFIER { 
        $$ = ast_new_range($2, ast_new_identifier($5)); 
    }
    | LPAREN expr RPAREN RANGE LPAREN expr RPAREN { 
        $$ = ast_new_range($2, $6); 
    }
    ;

/* Wand call: /func/arg1/arg2 */
wand_call
    : SLASH IDENTIFIER wand_args { 
        $$ = ast_new_wand_call($2, $3); 
    }
    | SLASH IDENTIFIER SLASH { 
        /* /func/ with no args (trailing slash) */
        $$ = ast_new_wand_call($2, ast_list_new()); 
    }
    | SLASH IDENTIFIER { 
        $$ = ast_new_wand_call($2, ast_list_new()); 
    }
    ;

wand_args
    : SLASH wand_arg { $$ = ast_list_new(); ast_list_append($$, $2); }
    | wand_args SLASH wand_arg { ast_list_append($1, $3); $$ = $1; }
    ;

/* Simple expressions allowed as wand arguments */
wand_arg
    : INT_LITERAL { $$ = ast_new_int_literal($1); }
    | FLOAT_LITERAL { $$ = ast_new_float_literal($1); }
    | STRING_LITERAL { $$ = ast_new_string_literal($1); }
    | TRUE { $$ = ast_new_bool_literal(1); }
    | FALSE { $$ = ast_new_bool_literal(0); }
    | IDENTIFIER { $$ = ast_new_identifier($1); }
    | UNDERSCORE { $$ = ast_new_implicit(); }
    | MINUS wand_arg { $$ = ast_new_unary(OP_NEG, $2); }
    | LPAREN expr RPAREN { $$ = $2; }
    | wand_arg LBRACKET expr RBRACKET { $$ = ast_new_index($1, $3); }
    | wand_arg MEMBER_ACCESS IDENTIFIER { $$ = ast_new_member($1, $3); }
    ;

lambda
    : BACKSLASH LPAREN param_list_opt RPAREN ARROW expr {
        $$ = ast_new_lambda($3, $6);
    }
    | BACKSLASH LPAREN param_list_opt RPAREN block {
        $$ = ast_new_lambda($3, $5);
    }
    ;

pattern_match
    : LBRACE match_arms RBRACE { $$ = ast_new_match($2); }
    ;

match_arms
    : match_arm { $$ = ast_list_new(); ast_list_append($$, $1); }
    | match_arms match_arm { ast_list_append($1, $2); $$ = $1; }
    ;

match_arm
    : expr ARROW expr { $$ = ast_new_match_arm($1, $3); }
    | UNDERSCORE ARROW expr { $$ = ast_new_match_arm(ast_new_implicit(), $3); }
    ;

expr_list_opt
    : /* empty */ { $$ = ast_list_new(); }
    | expr_list { $$ = $1; }
    ;

expr_list
    : expr { $$ = ast_list_new(); ast_list_append($$, $1); }
    | expr_list COMMA expr { ast_list_append($1, $3); $$ = $1; }
    ;

array_items
    : expr { $$ = ast_list_new(); ast_list_append($$, $1); }
    | array_items COMMA expr { ast_list_append($1, $3); $$ = $1; }
    ;

object_fields_opt
    : /* empty */ { $$ = ast_list_new(); }
    | object_fields { $$ = $1; }
    ;

object_fields
    : object_field { $$ = ast_list_new(); ast_list_append($$, $1); }
    | object_fields COMMA object_field { ast_list_append($1, $3); $$ = $1; }
    ;

object_field
    : IDENTIFIER COLON expr { $$ = ast_new_object_field($1, $3); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}
