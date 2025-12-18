/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INT_LITERAL = 258,             /* INT_LITERAL  */
    FLOAT_LITERAL = 259,           /* FLOAT_LITERAL  */
    IDENTIFIER = 260,              /* IDENTIFIER  */
    STRING_LITERAL = 261,          /* STRING_LITERAL  */
    LOOP = 262,                    /* LOOP  */
    FOR = 263,                     /* FOR  */
    IN = 264,                      /* IN  */
    IF = 265,                      /* IF  */
    ELSE = 266,                    /* ELSE  */
    WHEN = 267,                    /* WHEN  */
    UNLESS = 268,                  /* UNLESS  */
    AND = 269,                     /* AND  */
    OR = 270,                      /* OR  */
    NOT = 271,                     /* NOT  */
    TRUE = 272,                    /* TRUE  */
    FALSE = 273,                   /* FALSE  */
    ASSIGN_DECL = 274,             /* ASSIGN_DECL  */
    RETURN = 275,                  /* RETURN  */
    BREAK = 276,                   /* BREAK  */
    ARROW = 277,                   /* ARROW  */
    RANGE = 278,                   /* RANGE  */
    MEMBER_ACCESS = 279,           /* MEMBER_ACCESS  */
    EQ = 280,                      /* EQ  */
    NE = 281,                      /* NE  */
    LE = 282,                      /* LE  */
    GE = 283,                      /* GE  */
    LT = 284,                      /* LT  */
    GT = 285,                      /* GT  */
    DOT = 286,                     /* DOT  */
    COMMA = 287,                   /* COMMA  */
    COLON = 288,                   /* COLON  */
    ASSIGN = 289,                  /* ASSIGN  */
    PLUS = 290,                    /* PLUS  */
    MINUS = 291,                   /* MINUS  */
    STAR = 292,                    /* STAR  */
    SLASH = 293,                   /* SLASH  */
    PERCENT = 294,                 /* PERCENT  */
    LPAREN = 295,                  /* LPAREN  */
    RPAREN = 296,                  /* RPAREN  */
    LBRACE = 297,                  /* LBRACE  */
    RBRACE = 298,                  /* RBRACE  */
    LBRACKET = 299,                /* LBRACKET  */
    RBRACKET = 300,                /* RBRACKET  */
    PIPE = 301,                    /* PIPE  */
    HASH = 302,                    /* HASH  */
    BACKSLASH = 303,               /* BACKSLASH  */
    UNDERSCORE = 304,              /* UNDERSCORE  */
    OBJ_OPEN = 305,                /* OBJ_OPEN  */
    OBJ_CLOSE = 306,               /* OBJ_CLOSE  */
    USE = 307,                     /* USE  */
    UMINUS = 308                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "parser.y"

    int ival;
    double fval;
    char *sval;
    struct ASTNode *node;
    struct ASTList *list;

#line 125 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
