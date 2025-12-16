#ifndef AST_H
#define AST_H

#include <stddef.h>

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct ASTList ASTList;

// Node types
typedef enum {
  NODE_PROGRAM,
  NODE_FUNCTION,
  NODE_PARAM,
  NODE_BLOCK,
  NODE_VAR_DECL,
  NODE_LOOP,
  NODE_FOR,
  NODE_RETURN,
  NODE_BREAK,
  NODE_WHEN_STMT,
  NODE_EXPR_STMT,
  NODE_BINARY_OP,
  NODE_UNARY_OP,
  NODE_WAND_CALL,
  NODE_IDENTIFIER,
  NODE_INT_LITERAL,
  NODE_FLOAT_LITERAL,
  NODE_STRING_LITERAL,
  NODE_BOOL_LITERAL,
  NODE_ASSIGN,
  NODE_INDEX,
  NODE_IMPLICIT,
  NODE_ARRAY,
  NODE_OBJECT,
  NODE_RANGE,
  NODE_LAMBDA,
  NODE_MATCH,
  NODE_MATCH_ARM,
  NODE_PIPE,
  NODE_TERNARY,
  NODE_OBJECT_FIELD,
  NODE_MEMBER,
  NODE_USE,
} NodeType;

// Binary operators
typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_EQ,
  OP_NE,
  OP_LT,
  OP_GT,
  OP_LE,
  OP_GE,
  OP_AND,
  OP_OR,
} BinaryOp;

// Unary operators
typedef enum {
  OP_NEG,
  OP_NOT,
} UnaryOp;

// AST List (for function params, statements, etc.)
struct ASTList {
  ASTNode **items;
  size_t count;
  size_t capacity;
};

// AST Node
struct ASTNode {
  NodeType type;
  int line;
  int column;

  union {
    // NODE_PROGRAM
    struct {
      ASTList *decls;
    } program;

    // NODE_FUNCTION
    struct {
      char *name;
      ASTList *params;
      ASTNode *body;
    } function;

    // NODE_PARAM
    struct {
      char *name;
    } param;

    // NODE_BLOCK
    struct {
      ASTList *statements;
    } block;

    // NODE_VAR_DECL
    struct {
      char *name;
      ASTNode *init;
    } var_decl;

    // NODE_LOOP
    struct {
      ASTNode *condition; // NULL for infinite loop
      ASTNode *body;
    } loop;

    // NODE_FOR
    struct {
      char *var_name;
      ASTNode *iterable;
      ASTNode *body;
    } for_loop;

    // NODE_RETURN
    struct {
      ASTNode *value; // can be NULL
    } return_stmt;

    // NODE_BREAK
    struct {
      ASTNode *condition; // can be NULL
    } break_stmt;

    // NODE_WHEN_STMT
    struct {
      ASTNode *action;
      ASTNode *condition;
      int is_unless; // 1 for unless, 0 for when
    } when_stmt;

    // NODE_EXPR_STMT
    struct {
      ASTNode *expr;
    } expr_stmt;

    // NODE_BINARY_OP
    struct {
      BinaryOp op;
      ASTNode *left;
      ASTNode *right;
    } binary;

    // NODE_UNARY_OP
    struct {
      UnaryOp op;
      ASTNode *operand;
    } unary;

    // NODE_WAND_CALL
    struct {
      char *name;
      ASTList *args;
    } wand_call;

    // NODE_IDENTIFIER
    struct {
      char *name;
    } identifier;

    // NODE_INT_LITERAL
    struct {
      int value;
    } int_literal;

    // NODE_FLOAT_LITERAL
    struct {
      double value;
    } float_literal;

    // NODE_STRING_LITERAL
    struct {
      char *value;
    } string_literal;

    // NODE_BOOL_LITERAL
    struct {
      int value;
    } bool_literal;

    // NODE_ASSIGN
    struct {
      ASTNode *target;
      ASTNode *value;
    } assign;

    // NODE_INDEX
    struct {
      ASTNode *array;
      ASTNode *index;
    } index;

    // NODE_ARRAY
    struct {
      ASTList *elements;
    } array;

    // NODE_OBJECT
    struct {
      ASTList *fields;
    } object;

    // NODE_RANGE
    struct {
      ASTNode *start;
      ASTNode *end;
    } range;

    // NODE_LAMBDA
    struct {
      ASTList *params;
      ASTNode *body;
    } lambda;

    // NODE_MATCH
    struct {
      ASTList *arms;
    } match;

    // NODE_MATCH_ARM
    struct {
      ASTNode *pattern;
      ASTNode *body;
    } match_arm;

    // NODE_PIPE
    struct {
      ASTNode *left;
      ASTNode *right;
    } pipe;

    // NODE_TERNARY
    struct {
      ASTNode *condition;
      ASTNode *then_expr;
      ASTNode *else_expr;
    } ternary;

    // NODE_OBJECT_FIELD
    struct {
      char *key;
      ASTNode *value;
    } object_field;

    // NODE_MEMBER
    struct {
      ASTNode *object;
      char *member;
    } member;

    // NODE_USE
    struct {
      char *path;
    } use_stmt;
  } data;
};

// Constructor functions
ASTNode *ast_new_program(ASTList *decls);
ASTNode *ast_new_function(char *name, ASTList *params, ASTNode *body);
ASTNode *ast_new_param(char *name);
ASTNode *ast_new_block(ASTList *statements);
ASTNode *ast_new_var_decl(char *name, ASTNode *init);
ASTNode *ast_new_loop(ASTNode *condition, ASTNode *body);
ASTNode *ast_new_for(char *var_name, ASTNode *iterable, ASTNode *body);
ASTNode *ast_new_return(ASTNode *value);
ASTNode *ast_new_break(ASTNode *condition);
ASTNode *ast_new_when_stmt(ASTNode *action, ASTNode *condition, int is_unless);
ASTNode *ast_new_expr_stmt(ASTNode *expr);
ASTNode *ast_new_binary(BinaryOp op, ASTNode *left, ASTNode *right);
ASTNode *ast_new_unary(UnaryOp op, ASTNode *operand);
ASTNode *ast_new_wand_call(char *name, ASTList *args);
ASTNode *ast_new_identifier(char *name);
ASTNode *ast_new_int_literal(int value);
ASTNode *ast_new_float_literal(double value);
ASTNode *ast_new_string_literal(char *value);
ASTNode *ast_new_bool_literal(int value);
ASTNode *ast_new_assign(ASTNode *target, ASTNode *value);
ASTNode *ast_new_index(ASTNode *array, ASTNode *index);
ASTNode *ast_new_implicit(void);
ASTNode *ast_new_array(ASTList *elements);
ASTNode *ast_new_object(ASTList *fields);
ASTNode *ast_new_range(ASTNode *start, ASTNode *end);
ASTNode *ast_new_lambda(ASTList *params, ASTNode *body);
ASTNode *ast_new_match(ASTList *arms);
ASTNode *ast_new_match_arm(ASTNode *pattern, ASTNode *body);
ASTNode *ast_new_pipe(ASTNode *left, ASTNode *right);
ASTNode *ast_new_ternary(ASTNode *condition, ASTNode *then_expr,
                         ASTNode *else_expr);
ASTNode *ast_new_object_field(char *key, ASTNode *value);
ASTNode *ast_new_member(ASTNode *object, char *member);
ASTNode *ast_new_use(char *path);

// List functions
ASTList *ast_list_new(void);
void ast_list_append(ASTList *list, ASTNode *node);
void ast_list_prepend(ASTList *list, ASTNode *node);

// Debug printing
void ast_print(ASTNode *node, int indent);

// Global AST root (set by parser)
extern ASTNode *ast_root;

#endif // AST_H
