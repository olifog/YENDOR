/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

void yyerror(const char *s);

#line 85 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT_LITERAL = 3,                /* INT_LITERAL  */
  YYSYMBOL_FLOAT_LITERAL = 4,              /* FLOAT_LITERAL  */
  YYSYMBOL_IDENTIFIER = 5,                 /* IDENTIFIER  */
  YYSYMBOL_STRING_LITERAL = 6,             /* STRING_LITERAL  */
  YYSYMBOL_LOOP = 7,                       /* LOOP  */
  YYSYMBOL_FOR = 8,                        /* FOR  */
  YYSYMBOL_IN = 9,                         /* IN  */
  YYSYMBOL_IF = 10,                        /* IF  */
  YYSYMBOL_ELSE = 11,                      /* ELSE  */
  YYSYMBOL_WHEN = 12,                      /* WHEN  */
  YYSYMBOL_UNLESS = 13,                    /* UNLESS  */
  YYSYMBOL_AND = 14,                       /* AND  */
  YYSYMBOL_OR = 15,                        /* OR  */
  YYSYMBOL_NOT = 16,                       /* NOT  */
  YYSYMBOL_TRUE = 17,                      /* TRUE  */
  YYSYMBOL_FALSE = 18,                     /* FALSE  */
  YYSYMBOL_ASSIGN_DECL = 19,               /* ASSIGN_DECL  */
  YYSYMBOL_RETURN = 20,                    /* RETURN  */
  YYSYMBOL_BREAK = 21,                     /* BREAK  */
  YYSYMBOL_ARROW = 22,                     /* ARROW  */
  YYSYMBOL_RANGE = 23,                     /* RANGE  */
  YYSYMBOL_MEMBER_ACCESS = 24,             /* MEMBER_ACCESS  */
  YYSYMBOL_EQ = 25,                        /* EQ  */
  YYSYMBOL_NE = 26,                        /* NE  */
  YYSYMBOL_LE = 27,                        /* LE  */
  YYSYMBOL_GE = 28,                        /* GE  */
  YYSYMBOL_LT = 29,                        /* LT  */
  YYSYMBOL_GT = 30,                        /* GT  */
  YYSYMBOL_DOT = 31,                       /* DOT  */
  YYSYMBOL_COMMA = 32,                     /* COMMA  */
  YYSYMBOL_COLON = 33,                     /* COLON  */
  YYSYMBOL_ASSIGN = 34,                    /* ASSIGN  */
  YYSYMBOL_PLUS = 35,                      /* PLUS  */
  YYSYMBOL_MINUS = 36,                     /* MINUS  */
  YYSYMBOL_STAR = 37,                      /* STAR  */
  YYSYMBOL_SLASH = 38,                     /* SLASH  */
  YYSYMBOL_PERCENT = 39,                   /* PERCENT  */
  YYSYMBOL_LPAREN = 40,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 41,                    /* RPAREN  */
  YYSYMBOL_LBRACE = 42,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 43,                    /* RBRACE  */
  YYSYMBOL_LBRACKET = 44,                  /* LBRACKET  */
  YYSYMBOL_RBRACKET = 45,                  /* RBRACKET  */
  YYSYMBOL_PIPE = 46,                      /* PIPE  */
  YYSYMBOL_HASH = 47,                      /* HASH  */
  YYSYMBOL_BACKSLASH = 48,                 /* BACKSLASH  */
  YYSYMBOL_UNDERSCORE = 49,                /* UNDERSCORE  */
  YYSYMBOL_OBJ_OPEN = 50,                  /* OBJ_OPEN  */
  YYSYMBOL_OBJ_CLOSE = 51,                 /* OBJ_CLOSE  */
  YYSYMBOL_USE = 52,                       /* USE  */
  YYSYMBOL_UMINUS = 53,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 54,                  /* $accept  */
  YYSYMBOL_program = 55,                   /* program  */
  YYSYMBOL_top_level_list = 56,            /* top_level_list  */
  YYSYMBOL_top_level = 57,                 /* top_level  */
  YYSYMBOL_use_stmt = 58,                  /* use_stmt  */
  YYSYMBOL_func_def = 59,                  /* func_def  */
  YYSYMBOL_param_list_opt = 60,            /* param_list_opt  */
  YYSYMBOL_param_list = 61,                /* param_list  */
  YYSYMBOL_block = 62,                     /* block  */
  YYSYMBOL_statement_list = 63,            /* statement_list  */
  YYSYMBOL_statement = 64,                 /* statement  */
  YYSYMBOL_var_decl = 65,                  /* var_decl  */
  YYSYMBOL_assign_stmt = 66,               /* assign_stmt  */
  YYSYMBOL_loop_stmt = 67,                 /* loop_stmt  */
  YYSYMBOL_for_stmt = 68,                  /* for_stmt  */
  YYSYMBOL_expr = 69,                      /* expr  */
  YYSYMBOL_cond_expr = 70,                 /* cond_expr  */
  YYSYMBOL_or_expr = 71,                   /* or_expr  */
  YYSYMBOL_and_expr = 72,                  /* and_expr  */
  YYSYMBOL_eq_expr = 73,                   /* eq_expr  */
  YYSYMBOL_rel_expr = 74,                  /* rel_expr  */
  YYSYMBOL_add_expr = 75,                  /* add_expr  */
  YYSYMBOL_mul_expr = 76,                  /* mul_expr  */
  YYSYMBOL_unary_expr = 77,                /* unary_expr  */
  YYSYMBOL_postfix_expr = 78,              /* postfix_expr  */
  YYSYMBOL_primary_expr = 79,              /* primary_expr  */
  YYSYMBOL_wand_call = 80,                 /* wand_call  */
  YYSYMBOL_wand_args = 81,                 /* wand_args  */
  YYSYMBOL_wand_arg = 82,                  /* wand_arg  */
  YYSYMBOL_lambda = 83,                    /* lambda  */
  YYSYMBOL_pattern_match = 84,             /* pattern_match  */
  YYSYMBOL_match_arms = 85,                /* match_arms  */
  YYSYMBOL_match_arm = 86,                 /* match_arm  */
  YYSYMBOL_expr_list_opt = 87,             /* expr_list_opt  */
  YYSYMBOL_expr_list = 88,                 /* expr_list  */
  YYSYMBOL_object_fields_opt = 89,         /* object_fields_opt  */
  YYSYMBOL_object_fields = 90,             /* object_fields  */
  YYSYMBOL_object_field = 91               /* object_field  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   317

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  123
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  229

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   308


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    64,    64,    68,    69,    73,    74,    75,    79,    84,
      87,    93,    94,    98,    99,   103,   107,   108,   112,   113,
     114,   115,   116,   117,   120,   121,   122,   123,   124,   125,
     126,   130,   134,   135,   138,   141,   142,   145,   151,   152,
     156,   162,   163,   164,   168,   169,   175,   176,   180,   181,
     185,   186,   187,   191,   192,   193,   194,   195,   199,   200,
     201,   205,   206,   207,   208,   212,   213,   214,   218,   219,
     220,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   239,   242,   245,   248,   251,   254,
     257,   260,   267,   270,   274,   280,   281,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   300,   303,
     309,   313,   314,   318,   319,   323,   324,   328,   329,   338,
     339,   343,   344,   348
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INT_LITERAL",
  "FLOAT_LITERAL", "IDENTIFIER", "STRING_LITERAL", "LOOP", "FOR", "IN",
  "IF", "ELSE", "WHEN", "UNLESS", "AND", "OR", "NOT", "TRUE", "FALSE",
  "ASSIGN_DECL", "RETURN", "BREAK", "ARROW", "RANGE", "MEMBER_ACCESS",
  "EQ", "NE", "LE", "GE", "LT", "GT", "DOT", "COMMA", "COLON", "ASSIGN",
  "PLUS", "MINUS", "STAR", "SLASH", "PERCENT", "LPAREN", "RPAREN",
  "LBRACE", "RBRACE", "LBRACKET", "RBRACKET", "PIPE", "HASH", "BACKSLASH",
  "UNDERSCORE", "OBJ_OPEN", "OBJ_CLOSE", "USE", "UMINUS", "$accept",
  "program", "top_level_list", "top_level", "use_stmt", "func_def",
  "param_list_opt", "param_list", "block", "statement_list", "statement",
  "var_decl", "assign_stmt", "loop_stmt", "for_stmt", "expr", "cond_expr",
  "or_expr", "and_expr", "eq_expr", "rel_expr", "add_expr", "mul_expr",
  "unary_expr", "postfix_expr", "primary_expr", "wand_call", "wand_args",
  "wand_arg", "lambda", "pattern_match", "match_arms", "match_arm",
  "expr_list_opt", "expr_list", "object_fields_opt", "object_fields",
  "object_field", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-130)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -1,     1,    13,    39,    30,    -1,  -130,  -130,  -130,  -130,
     260,     3,    41,  -130,  -130,    43,  -130,    55,  -130,   260,
    -130,  -130,   260,   102,   260,   260,   124,  -130,   155,    45,
    -130,    95,   156,    91,   181,   168,   157,  -130,    -8,  -130,
    -130,  -130,   167,  -130,     8,    14,  -130,  -130,    89,    87,
     128,   141,   160,   167,   194,   138,   173,  -130,  -130,   131,
     260,   260,   260,   260,   260,   260,   260,   260,   260,   260,
     260,   260,   260,   260,   226,   260,  -130,   192,   203,  -130,
    -130,   260,  -130,  -130,   260,    20,   209,   228,  -130,   260,
     208,   260,  -130,   155,   267,  -130,  -130,    98,   156,    91,
     181,   181,   168,   168,   168,   168,   157,   157,  -130,  -130,
    -130,  -130,   178,    19,   248,   111,   120,  -130,  -130,  -130,
    -130,  -130,  -130,     4,   260,  -130,    49,     4,    24,   128,
      53,   128,  -130,   232,    17,   196,  -130,   260,  -130,   260,
    -130,  -130,  -130,  -130,  -130,    49,   137,   250,   260,    49,
    -130,  -130,   260,   260,  -130,   260,   260,  -130,  -130,  -130,
      65,    82,  -130,  -130,   180,   147,   128,   128,   128,  -130,
      78,    -7,   252,   212,     0,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,    21,    60,  -130,  -130,   260,   260,  -130,   249,
    -130,    16,   260,  -130,   260,   260,  -130,   260,    37,    73,
     260,   260,  -130,    75,    77,   108,    46,   260,   260,  -130,
    -130,    73,   109,  -130,  -130,  -130,   260,   260,  -130,   110,
     113,  -130,  -130,   119,   122,  -130,  -130,  -130,  -130
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     2,     3,     7,     5,     6,
       0,     0,     0,     1,     4,    71,    72,    76,    73,     0,
      74,    75,     0,     0,     0,   115,     0,    77,   119,     0,
      41,    44,    46,    48,    50,    53,    58,    61,    65,    68,
      79,    80,    11,     8,     0,     0,    67,    66,    94,     0,
     117,     0,   116,    11,     0,     0,   120,   121,    31,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,     0,    12,    83,
      84,     0,    86,    87,     0,    93,    92,    78,    81,     0,
       0,     0,    82,     0,     0,    42,    43,     0,    47,    49,
      51,    52,    56,    57,    54,    55,    59,    60,    62,    63,
      64,    70,     0,     0,     0,     0,     0,    97,    98,   102,
      99,   100,   101,     0,     0,   103,    95,     0,     0,   118,
       0,   123,   122,    77,     0,     0,   111,     0,    69,     0,
      16,     9,    14,    85,    88,   104,     0,     0,     0,    96,
      89,    90,     0,     0,   109,     0,     0,   110,   112,    45,
       0,     0,   105,   107,     0,     0,   108,   114,   113,    10,
      76,     0,     0,     0,     0,    15,    30,    17,    18,    19,
      20,    21,     0,    65,   106,    91,     0,     0,    38,     0,
      24,     0,     0,    25,     0,     0,    27,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,    32,
      39,     0,     0,    26,    28,    29,     0,     0,    35,     0,
       0,    40,    23,     0,     0,    33,    34,    36,    37
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -130,  -130,  -130,   254,  -130,  -130,   214,  -130,  -129,  -130,
    -130,   107,  -130,  -130,  -130,   -10,   -57,   215,   213,   207,
     174,   154,   172,   -16,   118,  -130,  -130,  -130,    35,  -130,
    -130,  -130,   145,  -130,  -130,  -130,  -130,   188
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     4,     5,     6,     7,     8,    77,    78,   141,   161,
     177,     9,   179,   180,   181,   134,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    86,   126,    41,
      96,   135,   136,    51,    52,    55,    56,    57
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      29,   154,    95,    46,     1,   187,    47,   117,   118,   119,
     120,    79,   192,    80,    49,    50,    74,    82,    11,    83,
      10,   121,   122,   117,   118,   119,   120,   150,   201,   151,
      13,   193,   176,   194,   195,   140,    75,   121,   122,   156,
     123,   139,   188,    42,   124,    12,     2,   202,    81,   207,
     208,     3,   196,   125,    84,   108,   109,   110,   216,   217,
     124,   140,    59,    59,   152,   112,    44,    59,   209,   125,
     210,   115,    43,   147,   116,   153,    58,   218,    45,   129,
     159,   131,   221,    59,    74,    15,    16,   170,    18,   171,
     172,    59,    59,   148,   197,   140,   169,    10,    19,    20,
      21,    45,   173,   174,    75,    60,   213,    48,   214,   137,
      61,    59,   186,    61,   146,   140,    63,    64,    22,    59,
      23,    59,    24,    59,   140,   175,    25,    85,    87,   160,
      26,    27,    28,    59,    15,    16,    17,    18,   164,   215,
     222,   225,   165,   166,   226,   167,   168,    19,    20,    21,
     227,   182,   143,   228,    59,    59,    59,    59,   145,    59,
      54,   144,   149,   191,    53,    59,    59,    22,    59,    23,
      62,    24,    76,    94,    59,    25,   198,   199,   162,    26,
      27,    28,   203,    59,   204,   205,    88,   206,   185,    92,
     211,   212,    89,    59,    71,    72,    73,   219,   220,    15,
      16,    17,    18,    69,    70,    93,   223,   224,    65,    66,
      67,    68,    19,    20,    21,    15,    16,    17,    18,   102,
     103,   104,   105,   138,    59,   184,    59,    91,    19,    20,
      21,   111,    22,   113,    23,   114,    24,   100,   101,   157,
      25,   106,   107,   190,    26,   133,    28,   127,    22,   130,
      23,   128,    24,   142,   155,   163,    25,   189,   200,    14,
      26,    27,    28,    15,    16,    17,    18,    90,   178,    99,
      15,    16,    17,    18,    98,    97,    19,    20,    21,   183,
     158,   132,     0,    19,    20,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,    23,     0,
      24,     0,     0,    22,    25,    23,     0,    24,    26,    27,
      28,    25,     0,     0,     0,    26,   133,    28
};

static const yytype_int16 yycheck[] =
{
      10,   130,    59,    19,     5,    12,    22,     3,     4,     5,
       6,     3,    12,     5,    24,    25,    24,     3,     5,     5,
      19,    17,    18,     3,     4,     5,     6,     3,    12,     5,
       0,    31,   161,    12,    13,    42,    44,    17,    18,    22,
      36,    22,   171,    40,    40,     6,    47,    31,    40,    12,
      13,    52,    31,    49,    40,    71,    72,    73,    12,    13,
      40,    42,    46,    46,    40,    75,    23,    46,    31,    49,
     199,    81,    31,    24,    84,    22,    31,    31,    23,    89,
     137,    91,   211,    46,    24,     3,     4,     5,     6,     7,
       8,    46,    46,    44,    34,    42,    31,    19,    16,    17,
      18,    23,    20,    21,    44,    10,    31,     5,    31,    11,
      15,    46,    34,    15,   124,    42,    25,    26,    36,    46,
      38,    46,    40,    46,    42,    43,    44,    38,    41,   139,
      48,    49,    50,    46,     3,     4,     5,     6,   148,    31,
      31,    31,   152,   153,    31,   155,   156,    16,    17,    18,
      31,   161,    41,    31,    46,    46,    46,    46,   123,    46,
       5,    41,   127,   173,    40,    46,    46,    36,    46,    38,
      14,    40,     5,    42,    46,    44,   186,   187,    41,    48,
      49,    50,   192,    46,   194,   195,    45,   197,    41,    51,
     200,   201,    32,    46,    37,    38,    39,   207,   208,     3,
       4,     5,     6,    35,    36,    32,   216,   217,    27,    28,
      29,    30,    16,    17,    18,     3,     4,     5,     6,    65,
      66,    67,    68,    45,    46,    45,    46,    33,    16,    17,
      18,     5,    36,    41,    38,    32,    40,    63,    64,    43,
      44,    69,    70,    31,    48,    49,    50,    38,    36,    41,
      38,    23,    40,     5,    22,     5,    44,     5,     9,     5,
      48,    49,    50,     3,     4,     5,     6,    53,   161,    62,
       3,     4,     5,     6,    61,    60,    16,    17,    18,   161,
     135,    93,    -1,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    38,    -1,
      40,    -1,    -1,    36,    44,    38,    -1,    40,    48,    49,
      50,    44,    -1,    -1,    -1,    48,    49,    50
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     5,    47,    52,    55,    56,    57,    58,    59,    65,
      19,     5,     6,     0,    57,     3,     4,     5,     6,    16,
      17,    18,    36,    38,    40,    44,    48,    49,    50,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    83,    40,    31,    23,    23,    77,    77,     5,    69,
      69,    87,    88,    40,     5,    89,    90,    91,    31,    46,
      10,    15,    14,    25,    26,    27,    28,    29,    30,    35,
      36,    37,    38,    39,    24,    44,     5,    60,    61,     3,
       5,    40,     3,     5,    40,    38,    81,    41,    45,    32,
      60,    33,    51,    32,    42,    70,    84,    71,    72,    73,
      74,    74,    75,    75,    75,    75,    76,    76,    77,    77,
      77,     5,    69,    41,    32,    69,    69,     3,     4,     5,
       6,    17,    18,    36,    40,    49,    82,    38,    23,    69,
      41,    69,    91,    49,    69,    85,    86,    11,    45,    22,
      42,    62,     5,    41,    41,    82,    69,    24,    44,    82,
       3,     5,    40,    22,    62,    22,    22,    43,    86,    70,
      69,    63,    41,     5,    69,    69,    69,    69,    69,    31,
       5,     7,     8,    20,    21,    43,    62,    64,    65,    66,
      67,    68,    69,    78,    45,    41,    34,    12,    62,     5,
      31,    69,    12,    31,    12,    13,    31,    34,    69,    69,
       9,    12,    31,    69,    69,    69,    69,    12,    13,    31,
      62,    69,    69,    31,    31,    31,    12,    13,    31,    69,
      69,    62,    31,    69,    69,    31,    31,    31,    31
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    54,    55,    56,    56,    57,    57,    57,    58,    59,
      59,    60,    60,    61,    61,    62,    63,    63,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    65,    66,    66,    66,    66,    66,    66,    67,    67,
      68,    69,    69,    69,    70,    70,    71,    71,    72,    72,
      73,    73,    73,    74,    74,    74,    74,    74,    75,    75,
      75,    76,    76,    76,    76,    77,    77,    77,    78,    78,
      78,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    80,    80,    80,    81,    81,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    83,    83,
      84,    85,    85,    86,    86,    87,    87,    88,    88,    89,
      89,    90,    90,    91
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     3,     6,
       8,     0,     1,     1,     3,     3,     0,     2,     1,     1,
       1,     1,     3,     5,     2,     2,     4,     2,     4,     4,
       1,     4,     4,     6,     6,     4,     6,     6,     2,     4,
       5,     1,     3,     3,     1,     5,     1,     3,     1,     3,
       1,     3,     3,     1,     3,     3,     3,     3,     1,     3,
       3,     1,     3,     3,     3,     1,     2,     2,     1,     4,
       3,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     3,     3,     3,     3,     5,     3,     3,     5,     5,
       5,     7,     3,     3,     2,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     4,     3,     6,     5,
       3,     1,     2,     3,     3,     0,     1,     1,     3,     0,
       1,     1,     3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: top_level_list  */
#line 64 "parser.y"
                     { ast_root = ast_new_program((yyvsp[0].list)); }
#line 1454 "parser.tab.c"
    break;

  case 3: /* top_level_list: top_level  */
#line 68 "parser.y"
                { (yyval.list) = ast_list_new(); ast_list_append((yyval.list), (yyvsp[0].node)); }
#line 1460 "parser.tab.c"
    break;

  case 4: /* top_level_list: top_level_list top_level  */
#line 69 "parser.y"
                               { ast_list_append((yyvsp[-1].list), (yyvsp[0].node)); (yyval.list) = (yyvsp[-1].list); }
#line 1466 "parser.tab.c"
    break;

  case 5: /* top_level: func_def  */
#line 73 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1472 "parser.tab.c"
    break;

  case 6: /* top_level: var_decl  */
#line 74 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1478 "parser.tab.c"
    break;

  case 7: /* top_level: use_stmt  */
#line 75 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1484 "parser.tab.c"
    break;

  case 8: /* use_stmt: USE STRING_LITERAL DOT  */
#line 79 "parser.y"
                             { (yyval.node) = ast_new_use((yyvsp[-1].sval)); }
#line 1490 "parser.tab.c"
    break;

  case 9: /* func_def: HASH IDENTIFIER LPAREN param_list_opt RPAREN block  */
#line 84 "parser.y"
                                                         {
        (yyval.node) = ast_new_function((yyvsp[-4].sval), (yyvsp[-2].list), (yyvsp[0].node));
    }
#line 1498 "parser.tab.c"
    break;

  case 10: /* func_def: HASH IDENTIFIER LPAREN param_list_opt RPAREN ARROW expr DOT  */
#line 87 "parser.y"
                                                                  {
        (yyval.node) = ast_new_function((yyvsp[-6].sval), (yyvsp[-4].list), ast_new_return((yyvsp[-1].node)));
    }
#line 1506 "parser.tab.c"
    break;

  case 11: /* param_list_opt: %empty  */
#line 93 "parser.y"
                  { (yyval.list) = ast_list_new(); }
#line 1512 "parser.tab.c"
    break;

  case 12: /* param_list_opt: param_list  */
#line 94 "parser.y"
                 { (yyval.list) = (yyvsp[0].list); }
#line 1518 "parser.tab.c"
    break;

  case 13: /* param_list: IDENTIFIER  */
#line 98 "parser.y"
                 { (yyval.list) = ast_list_new(); ast_list_append((yyval.list), ast_new_param((yyvsp[0].sval))); }
#line 1524 "parser.tab.c"
    break;

  case 14: /* param_list: param_list COMMA IDENTIFIER  */
#line 99 "parser.y"
                                  { ast_list_append((yyvsp[-2].list), ast_new_param((yyvsp[0].sval))); (yyval.list) = (yyvsp[-2].list); }
#line 1530 "parser.tab.c"
    break;

  case 15: /* block: LBRACE statement_list RBRACE  */
#line 103 "parser.y"
                                   { (yyval.node) = ast_new_block((yyvsp[-1].list)); }
#line 1536 "parser.tab.c"
    break;

  case 16: /* statement_list: %empty  */
#line 107 "parser.y"
                  { (yyval.list) = ast_list_new(); }
#line 1542 "parser.tab.c"
    break;

  case 17: /* statement_list: statement_list statement  */
#line 108 "parser.y"
                               { ast_list_append((yyvsp[-1].list), (yyvsp[0].node)); (yyval.list) = (yyvsp[-1].list); }
#line 1548 "parser.tab.c"
    break;

  case 18: /* statement: var_decl  */
#line 112 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1554 "parser.tab.c"
    break;

  case 19: /* statement: assign_stmt  */
#line 113 "parser.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1560 "parser.tab.c"
    break;

  case 20: /* statement: loop_stmt  */
#line 114 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 1566 "parser.tab.c"
    break;

  case 21: /* statement: for_stmt  */
#line 115 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1572 "parser.tab.c"
    break;

  case 22: /* statement: RETURN expr DOT  */
#line 116 "parser.y"
                      { (yyval.node) = ast_new_return((yyvsp[-1].node)); }
#line 1578 "parser.tab.c"
    break;

  case 23: /* statement: RETURN expr WHEN expr DOT  */
#line 117 "parser.y"
                                { 
        (yyval.node) = ast_new_when_stmt(ast_new_return((yyvsp[-3].node)), (yyvsp[-1].node), 0); 
    }
#line 1586 "parser.tab.c"
    break;

  case 24: /* statement: RETURN DOT  */
#line 120 "parser.y"
                 { (yyval.node) = ast_new_return(NULL); }
#line 1592 "parser.tab.c"
    break;

  case 25: /* statement: BREAK DOT  */
#line 121 "parser.y"
                { (yyval.node) = ast_new_break(NULL); }
#line 1598 "parser.tab.c"
    break;

  case 26: /* statement: BREAK WHEN expr DOT  */
#line 122 "parser.y"
                          { (yyval.node) = ast_new_break((yyvsp[-1].node)); }
#line 1604 "parser.tab.c"
    break;

  case 27: /* statement: expr DOT  */
#line 123 "parser.y"
               { (yyval.node) = ast_new_expr_stmt((yyvsp[-1].node)); }
#line 1610 "parser.tab.c"
    break;

  case 28: /* statement: expr WHEN expr DOT  */
#line 124 "parser.y"
                         { (yyval.node) = ast_new_when_stmt((yyvsp[-3].node), (yyvsp[-1].node), 0); }
#line 1616 "parser.tab.c"
    break;

  case 29: /* statement: expr UNLESS expr DOT  */
#line 125 "parser.y"
                           { (yyval.node) = ast_new_when_stmt((yyvsp[-3].node), (yyvsp[-1].node), 1); }
#line 1622 "parser.tab.c"
    break;

  case 30: /* statement: block  */
#line 126 "parser.y"
            { (yyval.node) = (yyvsp[0].node); }
#line 1628 "parser.tab.c"
    break;

  case 31: /* var_decl: IDENTIFIER ASSIGN_DECL expr DOT  */
#line 130 "parser.y"
                                      { (yyval.node) = ast_new_var_decl((yyvsp[-3].sval), (yyvsp[-1].node)); }
#line 1634 "parser.tab.c"
    break;

  case 32: /* assign_stmt: IDENTIFIER ASSIGN expr DOT  */
#line 134 "parser.y"
                                 { (yyval.node) = ast_new_assign(ast_new_identifier((yyvsp[-3].sval)), (yyvsp[-1].node)); }
#line 1640 "parser.tab.c"
    break;

  case 33: /* assign_stmt: IDENTIFIER ASSIGN expr WHEN expr DOT  */
#line 135 "parser.y"
                                           { 
        (yyval.node) = ast_new_when_stmt(ast_new_assign(ast_new_identifier((yyvsp[-5].sval)), (yyvsp[-3].node)), (yyvsp[-1].node), 0); 
    }
#line 1648 "parser.tab.c"
    break;

  case 34: /* assign_stmt: IDENTIFIER ASSIGN expr UNLESS expr DOT  */
#line 138 "parser.y"
                                             { 
        (yyval.node) = ast_new_when_stmt(ast_new_assign(ast_new_identifier((yyvsp[-5].sval)), (yyvsp[-3].node)), (yyvsp[-1].node), 1); 
    }
#line 1656 "parser.tab.c"
    break;

  case 35: /* assign_stmt: postfix_expr ASSIGN expr DOT  */
#line 141 "parser.y"
                                   { (yyval.node) = ast_new_assign((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 1662 "parser.tab.c"
    break;

  case 36: /* assign_stmt: postfix_expr ASSIGN expr WHEN expr DOT  */
#line 142 "parser.y"
                                             { 
        (yyval.node) = ast_new_when_stmt(ast_new_assign((yyvsp[-5].node), (yyvsp[-3].node)), (yyvsp[-1].node), 0); 
    }
#line 1670 "parser.tab.c"
    break;

  case 37: /* assign_stmt: postfix_expr ASSIGN expr UNLESS expr DOT  */
#line 145 "parser.y"
                                               { 
        (yyval.node) = ast_new_when_stmt(ast_new_assign((yyvsp[-5].node), (yyvsp[-3].node)), (yyvsp[-1].node), 1); 
    }
#line 1678 "parser.tab.c"
    break;

  case 38: /* loop_stmt: LOOP block  */
#line 151 "parser.y"
                 { (yyval.node) = ast_new_loop(NULL, (yyvsp[0].node)); }
#line 1684 "parser.tab.c"
    break;

  case 39: /* loop_stmt: LOOP WHEN expr block  */
#line 152 "parser.y"
                           { (yyval.node) = ast_new_loop((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1690 "parser.tab.c"
    break;

  case 40: /* for_stmt: FOR IDENTIFIER IN expr block  */
#line 156 "parser.y"
                                   {
        (yyval.node) = ast_new_for((yyvsp[-3].sval), (yyvsp[-1].node), (yyvsp[0].node));
    }
#line 1698 "parser.tab.c"
    break;

  case 41: /* expr: cond_expr  */
#line 162 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 1704 "parser.tab.c"
    break;

  case 42: /* expr: expr PIPE cond_expr  */
#line 163 "parser.y"
                          { (yyval.node) = ast_new_pipe((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1710 "parser.tab.c"
    break;

  case 43: /* expr: expr PIPE pattern_match  */
#line 164 "parser.y"
                              { (yyval.node) = ast_new_pipe((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1716 "parser.tab.c"
    break;

  case 44: /* cond_expr: or_expr  */
#line 168 "parser.y"
              { (yyval.node) = (yyvsp[0].node); }
#line 1722 "parser.tab.c"
    break;

  case 45: /* cond_expr: or_expr IF or_expr ELSE cond_expr  */
#line 169 "parser.y"
                                        {
        (yyval.node) = ast_new_ternary((yyvsp[-2].node), (yyvsp[-4].node), (yyvsp[0].node));
    }
#line 1730 "parser.tab.c"
    break;

  case 46: /* or_expr: and_expr  */
#line 175 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1736 "parser.tab.c"
    break;

  case 47: /* or_expr: or_expr OR and_expr  */
#line 176 "parser.y"
                          { (yyval.node) = ast_new_binary(OP_OR, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1742 "parser.tab.c"
    break;

  case 48: /* and_expr: eq_expr  */
#line 180 "parser.y"
              { (yyval.node) = (yyvsp[0].node); }
#line 1748 "parser.tab.c"
    break;

  case 49: /* and_expr: and_expr AND eq_expr  */
#line 181 "parser.y"
                           { (yyval.node) = ast_new_binary(OP_AND, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1754 "parser.tab.c"
    break;

  case 50: /* eq_expr: rel_expr  */
#line 185 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1760 "parser.tab.c"
    break;

  case 51: /* eq_expr: eq_expr EQ rel_expr  */
#line 186 "parser.y"
                          { (yyval.node) = ast_new_binary(OP_EQ, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1766 "parser.tab.c"
    break;

  case 52: /* eq_expr: eq_expr NE rel_expr  */
#line 187 "parser.y"
                          { (yyval.node) = ast_new_binary(OP_NE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1772 "parser.tab.c"
    break;

  case 53: /* rel_expr: add_expr  */
#line 191 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1778 "parser.tab.c"
    break;

  case 54: /* rel_expr: rel_expr LT add_expr  */
#line 192 "parser.y"
                           { (yyval.node) = ast_new_binary(OP_LT, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1784 "parser.tab.c"
    break;

  case 55: /* rel_expr: rel_expr GT add_expr  */
#line 193 "parser.y"
                           { (yyval.node) = ast_new_binary(OP_GT, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1790 "parser.tab.c"
    break;

  case 56: /* rel_expr: rel_expr LE add_expr  */
#line 194 "parser.y"
                           { (yyval.node) = ast_new_binary(OP_LE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1796 "parser.tab.c"
    break;

  case 57: /* rel_expr: rel_expr GE add_expr  */
#line 195 "parser.y"
                           { (yyval.node) = ast_new_binary(OP_GE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1802 "parser.tab.c"
    break;

  case 58: /* add_expr: mul_expr  */
#line 199 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1808 "parser.tab.c"
    break;

  case 59: /* add_expr: add_expr PLUS mul_expr  */
#line 200 "parser.y"
                             { (yyval.node) = ast_new_binary(OP_ADD, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1814 "parser.tab.c"
    break;

  case 60: /* add_expr: add_expr MINUS mul_expr  */
#line 201 "parser.y"
                              { (yyval.node) = ast_new_binary(OP_SUB, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1820 "parser.tab.c"
    break;

  case 61: /* mul_expr: unary_expr  */
#line 205 "parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1826 "parser.tab.c"
    break;

  case 62: /* mul_expr: mul_expr STAR unary_expr  */
#line 206 "parser.y"
                               { (yyval.node) = ast_new_binary(OP_MUL, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1832 "parser.tab.c"
    break;

  case 63: /* mul_expr: mul_expr SLASH unary_expr  */
#line 207 "parser.y"
                                { (yyval.node) = ast_new_binary(OP_DIV, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1838 "parser.tab.c"
    break;

  case 64: /* mul_expr: mul_expr PERCENT unary_expr  */
#line 208 "parser.y"
                                  { (yyval.node) = ast_new_binary(OP_MOD, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1844 "parser.tab.c"
    break;

  case 65: /* unary_expr: postfix_expr  */
#line 212 "parser.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 1850 "parser.tab.c"
    break;

  case 66: /* unary_expr: MINUS unary_expr  */
#line 213 "parser.y"
                                    { (yyval.node) = ast_new_unary(OP_NEG, (yyvsp[0].node)); }
#line 1856 "parser.tab.c"
    break;

  case 67: /* unary_expr: NOT unary_expr  */
#line 214 "parser.y"
                     { (yyval.node) = ast_new_unary(OP_NOT, (yyvsp[0].node)); }
#line 1862 "parser.tab.c"
    break;

  case 68: /* postfix_expr: primary_expr  */
#line 218 "parser.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 1868 "parser.tab.c"
    break;

  case 69: /* postfix_expr: postfix_expr LBRACKET expr RBRACKET  */
#line 219 "parser.y"
                                          { (yyval.node) = ast_new_index((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 1874 "parser.tab.c"
    break;

  case 70: /* postfix_expr: postfix_expr MEMBER_ACCESS IDENTIFIER  */
#line 220 "parser.y"
                                            { (yyval.node) = ast_new_member((yyvsp[-2].node), (yyvsp[0].sval)); }
#line 1880 "parser.tab.c"
    break;

  case 71: /* primary_expr: INT_LITERAL  */
#line 224 "parser.y"
                  { (yyval.node) = ast_new_int_literal((yyvsp[0].ival)); }
#line 1886 "parser.tab.c"
    break;

  case 72: /* primary_expr: FLOAT_LITERAL  */
#line 225 "parser.y"
                    { (yyval.node) = ast_new_float_literal((yyvsp[0].fval)); }
#line 1892 "parser.tab.c"
    break;

  case 73: /* primary_expr: STRING_LITERAL  */
#line 226 "parser.y"
                     { (yyval.node) = ast_new_string_literal((yyvsp[0].sval)); }
#line 1898 "parser.tab.c"
    break;

  case 74: /* primary_expr: TRUE  */
#line 227 "parser.y"
           { (yyval.node) = ast_new_bool_literal(1); }
#line 1904 "parser.tab.c"
    break;

  case 75: /* primary_expr: FALSE  */
#line 228 "parser.y"
            { (yyval.node) = ast_new_bool_literal(0); }
#line 1910 "parser.tab.c"
    break;

  case 76: /* primary_expr: IDENTIFIER  */
#line 229 "parser.y"
                 { (yyval.node) = ast_new_identifier((yyvsp[0].sval)); }
#line 1916 "parser.tab.c"
    break;

  case 77: /* primary_expr: UNDERSCORE  */
#line 230 "parser.y"
                 { (yyval.node) = ast_new_implicit(); }
#line 1922 "parser.tab.c"
    break;

  case 78: /* primary_expr: LPAREN expr RPAREN  */
#line 231 "parser.y"
                         { (yyval.node) = (yyvsp[-1].node); }
#line 1928 "parser.tab.c"
    break;

  case 79: /* primary_expr: wand_call  */
#line 232 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 1934 "parser.tab.c"
    break;

  case 80: /* primary_expr: lambda  */
#line 233 "parser.y"
             { (yyval.node) = (yyvsp[0].node); }
#line 1940 "parser.tab.c"
    break;

  case 81: /* primary_expr: LBRACKET expr_list_opt RBRACKET  */
#line 234 "parser.y"
                                      { (yyval.node) = ast_new_array((yyvsp[-1].list)); }
#line 1946 "parser.tab.c"
    break;

  case 82: /* primary_expr: OBJ_OPEN object_fields_opt OBJ_CLOSE  */
#line 235 "parser.y"
                                           { (yyval.node) = ast_new_object((yyvsp[-1].list)); }
#line 1952 "parser.tab.c"
    break;

  case 83: /* primary_expr: INT_LITERAL RANGE INT_LITERAL  */
#line 236 "parser.y"
                                    { 
        (yyval.node) = ast_new_range(ast_new_int_literal((yyvsp[-2].ival)), ast_new_int_literal((yyvsp[0].ival))); 
    }
#line 1960 "parser.tab.c"
    break;

  case 84: /* primary_expr: INT_LITERAL RANGE IDENTIFIER  */
#line 239 "parser.y"
                                   { 
        (yyval.node) = ast_new_range(ast_new_int_literal((yyvsp[-2].ival)), ast_new_identifier((yyvsp[0].sval))); 
    }
#line 1968 "parser.tab.c"
    break;

  case 85: /* primary_expr: INT_LITERAL RANGE LPAREN expr RPAREN  */
#line 242 "parser.y"
                                           { 
        (yyval.node) = ast_new_range(ast_new_int_literal((yyvsp[-4].ival)), (yyvsp[-1].node)); 
    }
#line 1976 "parser.tab.c"
    break;

  case 86: /* primary_expr: IDENTIFIER RANGE INT_LITERAL  */
#line 245 "parser.y"
                                   { 
        (yyval.node) = ast_new_range(ast_new_identifier((yyvsp[-2].sval)), ast_new_int_literal((yyvsp[0].ival))); 
    }
#line 1984 "parser.tab.c"
    break;

  case 87: /* primary_expr: IDENTIFIER RANGE IDENTIFIER  */
#line 248 "parser.y"
                                  { 
        (yyval.node) = ast_new_range(ast_new_identifier((yyvsp[-2].sval)), ast_new_identifier((yyvsp[0].sval))); 
    }
#line 1992 "parser.tab.c"
    break;

  case 88: /* primary_expr: IDENTIFIER RANGE LPAREN expr RPAREN  */
#line 251 "parser.y"
                                          { 
        (yyval.node) = ast_new_range(ast_new_identifier((yyvsp[-4].sval)), (yyvsp[-1].node)); 
    }
#line 2000 "parser.tab.c"
    break;

  case 89: /* primary_expr: LPAREN expr RPAREN RANGE INT_LITERAL  */
#line 254 "parser.y"
                                           { 
        (yyval.node) = ast_new_range((yyvsp[-3].node), ast_new_int_literal((yyvsp[0].ival))); 
    }
#line 2008 "parser.tab.c"
    break;

  case 90: /* primary_expr: LPAREN expr RPAREN RANGE IDENTIFIER  */
#line 257 "parser.y"
                                          { 
        (yyval.node) = ast_new_range((yyvsp[-3].node), ast_new_identifier((yyvsp[0].sval))); 
    }
#line 2016 "parser.tab.c"
    break;

  case 91: /* primary_expr: LPAREN expr RPAREN RANGE LPAREN expr RPAREN  */
#line 260 "parser.y"
                                                  { 
        (yyval.node) = ast_new_range((yyvsp[-5].node), (yyvsp[-1].node)); 
    }
#line 2024 "parser.tab.c"
    break;

  case 92: /* wand_call: SLASH IDENTIFIER wand_args  */
#line 267 "parser.y"
                                 { 
        (yyval.node) = ast_new_wand_call((yyvsp[-1].sval), (yyvsp[0].list)); 
    }
#line 2032 "parser.tab.c"
    break;

  case 93: /* wand_call: SLASH IDENTIFIER SLASH  */
#line 270 "parser.y"
                             { 
        /* /func/ with no args (trailing slash) */
        (yyval.node) = ast_new_wand_call((yyvsp[-1].sval), ast_list_new()); 
    }
#line 2041 "parser.tab.c"
    break;

  case 94: /* wand_call: SLASH IDENTIFIER  */
#line 274 "parser.y"
                       { 
        (yyval.node) = ast_new_wand_call((yyvsp[0].sval), ast_list_new()); 
    }
#line 2049 "parser.tab.c"
    break;

  case 95: /* wand_args: SLASH wand_arg  */
#line 280 "parser.y"
                     { (yyval.list) = ast_list_new(); ast_list_append((yyval.list), (yyvsp[0].node)); }
#line 2055 "parser.tab.c"
    break;

  case 96: /* wand_args: wand_args SLASH wand_arg  */
#line 281 "parser.y"
                               { ast_list_append((yyvsp[-2].list), (yyvsp[0].node)); (yyval.list) = (yyvsp[-2].list); }
#line 2061 "parser.tab.c"
    break;

  case 97: /* wand_arg: INT_LITERAL  */
#line 286 "parser.y"
                  { (yyval.node) = ast_new_int_literal((yyvsp[0].ival)); }
#line 2067 "parser.tab.c"
    break;

  case 98: /* wand_arg: FLOAT_LITERAL  */
#line 287 "parser.y"
                    { (yyval.node) = ast_new_float_literal((yyvsp[0].fval)); }
#line 2073 "parser.tab.c"
    break;

  case 99: /* wand_arg: STRING_LITERAL  */
#line 288 "parser.y"
                     { (yyval.node) = ast_new_string_literal((yyvsp[0].sval)); }
#line 2079 "parser.tab.c"
    break;

  case 100: /* wand_arg: TRUE  */
#line 289 "parser.y"
           { (yyval.node) = ast_new_bool_literal(1); }
#line 2085 "parser.tab.c"
    break;

  case 101: /* wand_arg: FALSE  */
#line 290 "parser.y"
            { (yyval.node) = ast_new_bool_literal(0); }
#line 2091 "parser.tab.c"
    break;

  case 102: /* wand_arg: IDENTIFIER  */
#line 291 "parser.y"
                 { (yyval.node) = ast_new_identifier((yyvsp[0].sval)); }
#line 2097 "parser.tab.c"
    break;

  case 103: /* wand_arg: UNDERSCORE  */
#line 292 "parser.y"
                 { (yyval.node) = ast_new_implicit(); }
#line 2103 "parser.tab.c"
    break;

  case 104: /* wand_arg: MINUS wand_arg  */
#line 293 "parser.y"
                     { (yyval.node) = ast_new_unary(OP_NEG, (yyvsp[0].node)); }
#line 2109 "parser.tab.c"
    break;

  case 105: /* wand_arg: LPAREN expr RPAREN  */
#line 294 "parser.y"
                         { (yyval.node) = (yyvsp[-1].node); }
#line 2115 "parser.tab.c"
    break;

  case 106: /* wand_arg: wand_arg LBRACKET expr RBRACKET  */
#line 295 "parser.y"
                                      { (yyval.node) = ast_new_index((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2121 "parser.tab.c"
    break;

  case 107: /* wand_arg: wand_arg MEMBER_ACCESS IDENTIFIER  */
#line 296 "parser.y"
                                        { (yyval.node) = ast_new_member((yyvsp[-2].node), (yyvsp[0].sval)); }
#line 2127 "parser.tab.c"
    break;

  case 108: /* lambda: BACKSLASH LPAREN param_list_opt RPAREN ARROW expr  */
#line 300 "parser.y"
                                                        {
        (yyval.node) = ast_new_lambda((yyvsp[-3].list), (yyvsp[0].node));
    }
#line 2135 "parser.tab.c"
    break;

  case 109: /* lambda: BACKSLASH LPAREN param_list_opt RPAREN block  */
#line 303 "parser.y"
                                                   {
        (yyval.node) = ast_new_lambda((yyvsp[-2].list), (yyvsp[0].node));
    }
#line 2143 "parser.tab.c"
    break;

  case 110: /* pattern_match: LBRACE match_arms RBRACE  */
#line 309 "parser.y"
                               { (yyval.node) = ast_new_match((yyvsp[-1].list)); }
#line 2149 "parser.tab.c"
    break;

  case 111: /* match_arms: match_arm  */
#line 313 "parser.y"
                { (yyval.list) = ast_list_new(); ast_list_append((yyval.list), (yyvsp[0].node)); }
#line 2155 "parser.tab.c"
    break;

  case 112: /* match_arms: match_arms match_arm  */
#line 314 "parser.y"
                           { ast_list_append((yyvsp[-1].list), (yyvsp[0].node)); (yyval.list) = (yyvsp[-1].list); }
#line 2161 "parser.tab.c"
    break;

  case 113: /* match_arm: expr ARROW expr  */
#line 318 "parser.y"
                      { (yyval.node) = ast_new_match_arm((yyvsp[-2].node), (yyvsp[0].node)); }
#line 2167 "parser.tab.c"
    break;

  case 114: /* match_arm: UNDERSCORE ARROW expr  */
#line 319 "parser.y"
                            { (yyval.node) = ast_new_match_arm(ast_new_implicit(), (yyvsp[0].node)); }
#line 2173 "parser.tab.c"
    break;

  case 115: /* expr_list_opt: %empty  */
#line 323 "parser.y"
                  { (yyval.list) = ast_list_new(); }
#line 2179 "parser.tab.c"
    break;

  case 116: /* expr_list_opt: expr_list  */
#line 324 "parser.y"
                { (yyval.list) = (yyvsp[0].list); }
#line 2185 "parser.tab.c"
    break;

  case 117: /* expr_list: expr  */
#line 328 "parser.y"
           { (yyval.list) = ast_list_new(); ast_list_append((yyval.list), (yyvsp[0].node)); }
#line 2191 "parser.tab.c"
    break;

  case 118: /* expr_list: expr_list COMMA expr  */
#line 329 "parser.y"
                           { ast_list_append((yyvsp[-2].list), (yyvsp[0].node)); (yyval.list) = (yyvsp[-2].list); }
#line 2197 "parser.tab.c"
    break;

  case 119: /* object_fields_opt: %empty  */
#line 338 "parser.y"
                  { (yyval.list) = ast_list_new(); }
#line 2203 "parser.tab.c"
    break;

  case 120: /* object_fields_opt: object_fields  */
#line 339 "parser.y"
                    { (yyval.list) = (yyvsp[0].list); }
#line 2209 "parser.tab.c"
    break;

  case 121: /* object_fields: object_field  */
#line 343 "parser.y"
                   { (yyval.list) = ast_list_new(); ast_list_append((yyval.list), (yyvsp[0].node)); }
#line 2215 "parser.tab.c"
    break;

  case 122: /* object_fields: object_fields COMMA object_field  */
#line 344 "parser.y"
                                       { ast_list_append((yyvsp[-2].list), (yyvsp[0].node)); (yyval.list) = (yyvsp[-2].list); }
#line 2221 "parser.tab.c"
    break;

  case 123: /* object_field: IDENTIFIER COLON expr  */
#line 348 "parser.y"
                            { (yyval.node) = ast_new_object_field((yyvsp[-2].sval), (yyvsp[0].node)); }
#line 2227 "parser.tab.c"
    break;


#line 2231 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 351 "parser.y"


void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}
