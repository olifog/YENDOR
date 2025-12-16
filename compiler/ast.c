#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *ast_root = NULL;

static ASTNode *alloc_node(NodeType type) {
  ASTNode *node = calloc(1, sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  node->type = type;
  return node;
}

ASTNode *ast_new_program(ASTList *decls) {
  ASTNode *node = alloc_node(NODE_PROGRAM);
  node->data.program.decls = decls;
  return node;
}

ASTNode *ast_new_function(char *name, ASTList *params, ASTNode *body) {
  ASTNode *node = alloc_node(NODE_FUNCTION);
  node->data.function.name = name;
  node->data.function.params = params;
  node->data.function.body = body;
  return node;
}

ASTNode *ast_new_param(char *name) {
  ASTNode *node = alloc_node(NODE_PARAM);
  node->data.param.name = name;
  return node;
}

ASTNode *ast_new_block(ASTList *statements) {
  ASTNode *node = alloc_node(NODE_BLOCK);
  node->data.block.statements = statements;
  return node;
}

ASTNode *ast_new_var_decl(char *name, ASTNode *init) {
  ASTNode *node = alloc_node(NODE_VAR_DECL);
  node->data.var_decl.name = name;
  node->data.var_decl.init = init;
  return node;
}

ASTNode *ast_new_loop(ASTNode *condition, ASTNode *body) {
  ASTNode *node = alloc_node(NODE_LOOP);
  node->data.loop.condition = condition;
  node->data.loop.body = body;
  return node;
}

ASTNode *ast_new_for(char *var_name, ASTNode *iterable, ASTNode *body) {
  ASTNode *node = alloc_node(NODE_FOR);
  node->data.for_loop.var_name = var_name;
  node->data.for_loop.iterable = iterable;
  node->data.for_loop.body = body;
  return node;
}

ASTNode *ast_new_return(ASTNode *value) {
  ASTNode *node = alloc_node(NODE_RETURN);
  node->data.return_stmt.value = value;
  return node;
}

ASTNode *ast_new_break(ASTNode *condition) {
  ASTNode *node = alloc_node(NODE_BREAK);
  node->data.break_stmt.condition = condition;
  return node;
}

ASTNode *ast_new_when_stmt(ASTNode *action, ASTNode *condition, int is_unless) {
  ASTNode *node = alloc_node(NODE_WHEN_STMT);
  node->data.when_stmt.action = action;
  node->data.when_stmt.condition = condition;
  node->data.when_stmt.is_unless = is_unless;
  return node;
}

ASTNode *ast_new_expr_stmt(ASTNode *expr) {
  ASTNode *node = alloc_node(NODE_EXPR_STMT);
  node->data.expr_stmt.expr = expr;
  return node;
}

ASTNode *ast_new_binary(BinaryOp op, ASTNode *left, ASTNode *right) {
  ASTNode *node = alloc_node(NODE_BINARY_OP);
  node->data.binary.op = op;
  node->data.binary.left = left;
  node->data.binary.right = right;
  return node;
}

ASTNode *ast_new_unary(UnaryOp op, ASTNode *operand) {
  ASTNode *node = alloc_node(NODE_UNARY_OP);
  node->data.unary.op = op;
  node->data.unary.operand = operand;
  return node;
}

ASTNode *ast_new_wand_call(char *name, ASTList *args) {
  ASTNode *node = alloc_node(NODE_WAND_CALL);
  node->data.wand_call.name = name;
  node->data.wand_call.args = args;
  return node;
}

ASTNode *ast_new_identifier(char *name) {
  ASTNode *node = alloc_node(NODE_IDENTIFIER);
  node->data.identifier.name = name;
  return node;
}

ASTNode *ast_new_int_literal(int value) {
  ASTNode *node = alloc_node(NODE_INT_LITERAL);
  node->data.int_literal.value = value;
  return node;
}

ASTNode *ast_new_float_literal(double value) {
  ASTNode *node = alloc_node(NODE_FLOAT_LITERAL);
  node->data.float_literal.value = value;
  return node;
}

ASTNode *ast_new_string_literal(char *value) {
  ASTNode *node = alloc_node(NODE_STRING_LITERAL);
  node->data.string_literal.value = value;
  return node;
}

ASTNode *ast_new_bool_literal(int value) {
  ASTNode *node = alloc_node(NODE_BOOL_LITERAL);
  node->data.bool_literal.value = value;
  return node;
}

ASTNode *ast_new_assign(ASTNode *target, ASTNode *value) {
  ASTNode *node = alloc_node(NODE_ASSIGN);
  node->data.assign.target = target;
  node->data.assign.value = value;
  return node;
}

ASTNode *ast_new_index(ASTNode *array, ASTNode *index) {
  ASTNode *node = alloc_node(NODE_INDEX);
  node->data.index.array = array;
  node->data.index.index = index;
  return node;
}

ASTNode *ast_new_implicit(void) { return alloc_node(NODE_IMPLICIT); }

ASTNode *ast_new_array(ASTList *elements) {
  ASTNode *node = alloc_node(NODE_ARRAY);
  node->data.array.elements = elements;
  return node;
}

ASTNode *ast_new_object(ASTList *fields) {
  ASTNode *node = alloc_node(NODE_OBJECT);
  node->data.object.fields = fields;
  return node;
}

ASTNode *ast_new_range(ASTNode *start, ASTNode *end) {
  ASTNode *node = alloc_node(NODE_RANGE);
  node->data.range.start = start;
  node->data.range.end = end;
  return node;
}

ASTNode *ast_new_lambda(ASTList *params, ASTNode *body) {
  ASTNode *node = alloc_node(NODE_LAMBDA);
  node->data.lambda.params = params;
  node->data.lambda.body = body;
  return node;
}

ASTNode *ast_new_match(ASTList *arms) {
  ASTNode *node = alloc_node(NODE_MATCH);
  node->data.match.arms = arms;
  return node;
}

ASTNode *ast_new_match_arm(ASTNode *pattern, ASTNode *body) {
  ASTNode *node = alloc_node(NODE_MATCH_ARM);
  node->data.match_arm.pattern = pattern;
  node->data.match_arm.body = body;
  return node;
}

ASTNode *ast_new_pipe(ASTNode *left, ASTNode *right) {
  ASTNode *node = alloc_node(NODE_PIPE);
  node->data.pipe.left = left;
  node->data.pipe.right = right;
  return node;
}

ASTNode *ast_new_ternary(ASTNode *condition, ASTNode *then_expr,
                         ASTNode *else_expr) {
  ASTNode *node = alloc_node(NODE_TERNARY);
  node->data.ternary.condition = condition;
  node->data.ternary.then_expr = then_expr;
  node->data.ternary.else_expr = else_expr;
  return node;
}

ASTNode *ast_new_object_field(char *key, ASTNode *value) {
  ASTNode *node = alloc_node(NODE_OBJECT_FIELD);
  node->data.object_field.key = key;
  node->data.object_field.value = value;
  return node;
}

ASTNode *ast_new_member(ASTNode *object, char *member) {
  ASTNode *node = alloc_node(NODE_MEMBER);
  node->data.member.object = object;
  node->data.member.member = member;
  return node;
}

ASTNode *ast_new_use(char *path) {
  ASTNode *node = alloc_node(NODE_USE);
  node->data.use_stmt.path = path;
  return node;
}

// List functions
ASTList *ast_list_new(void) {
  ASTList *list = calloc(1, sizeof(ASTList));
  if (!list) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  list->capacity = 8;
  list->items = calloc(list->capacity, sizeof(ASTNode *));
  return list;
}

void ast_list_append(ASTList *list, ASTNode *node) {
  if (list->count >= list->capacity) {
    list->capacity *= 2;
    list->items = realloc(list->items, list->capacity * sizeof(ASTNode *));
  }
  list->items[list->count++] = node;
}

void ast_list_prepend(ASTList *list, ASTNode *node) {
  if (list->count >= list->capacity) {
    list->capacity *= 2;
    list->items = realloc(list->items, list->capacity * sizeof(ASTNode *));
  }
  // Shift all elements right
  for (size_t i = list->count; i > 0; i--) {
    list->items[i] = list->items[i - 1];
  }
  list->items[0] = node;
  list->count++;
}

// Debug printing
static void print_indent(int indent) {
  for (int i = 0; i < indent; i++)
    printf("  ");
}

static const char *binop_to_str(BinaryOp op) {
  switch (op) {
  case OP_ADD:
    return "+";
  case OP_SUB:
    return "-";
  case OP_MUL:
    return "*";
  case OP_DIV:
    return "/";
  case OP_MOD:
    return "%";
  case OP_EQ:
    return "==";
  case OP_NE:
    return "!=";
  case OP_LT:
    return "<";
  case OP_GT:
    return ">";
  case OP_LE:
    return "<=";
  case OP_GE:
    return ">=";
  case OP_AND:
    return "and";
  case OP_OR:
    return "or";
  default:
    return "?";
  }
}

static const char *unop_to_str(UnaryOp op) {
  switch (op) {
  case OP_NEG:
    return "-";
  case OP_NOT:
    return "not";
  default:
    return "?";
  }
}

void ast_print(ASTNode *node, int indent) {
  if (!node)
    return;

  print_indent(indent);

  switch (node->type) {
  case NODE_PROGRAM:
    printf("Program\n");
    if (node->data.program.decls) {
      for (size_t i = 0; i < node->data.program.decls->count; i++) {
        ast_print(node->data.program.decls->items[i], indent + 1);
      }
    }
    break;

  case NODE_FUNCTION:
    printf("Function: #%s\n", node->data.function.name);
    if (node->data.function.params && node->data.function.params->count > 0) {
      print_indent(indent + 1);
      printf("Params:\n");
      for (size_t i = 0; i < node->data.function.params->count; i++) {
        ast_print(node->data.function.params->items[i], indent + 2);
      }
    }
    ast_print(node->data.function.body, indent + 1);
    break;

  case NODE_PARAM:
    printf("Param: %s\n", node->data.param.name);
    break;

  case NODE_BLOCK:
    printf("Block\n");
    if (node->data.block.statements) {
      for (size_t i = 0; i < node->data.block.statements->count; i++) {
        ast_print(node->data.block.statements->items[i], indent + 1);
      }
    }
    break;

  case NODE_VAR_DECL:
    printf("VarDecl: %s\n", node->data.var_decl.name);
    if (node->data.var_decl.init) {
      ast_print(node->data.var_decl.init, indent + 1);
    }
    break;

  case NODE_LOOP:
    printf("Loop\n");
    if (node->data.loop.condition) {
      print_indent(indent + 1);
      printf("Condition:\n");
      ast_print(node->data.loop.condition, indent + 2);
    }
    ast_print(node->data.loop.body, indent + 1);
    break;

  case NODE_FOR:
    printf("For: %s\n", node->data.for_loop.var_name);
    print_indent(indent + 1);
    printf("In:\n");
    ast_print(node->data.for_loop.iterable, indent + 2);
    ast_print(node->data.for_loop.body, indent + 1);
    break;

  case NODE_RETURN:
    printf("Return\n");
    if (node->data.return_stmt.value) {
      ast_print(node->data.return_stmt.value, indent + 1);
    }
    break;

  case NODE_BREAK:
    printf("Break\n");
    if (node->data.break_stmt.condition) {
      ast_print(node->data.break_stmt.condition, indent + 1);
    }
    break;

  case NODE_WHEN_STMT:
    printf("%s\n", node->data.when_stmt.is_unless ? "Unless" : "When");
    ast_print(node->data.when_stmt.action, indent + 1);
    ast_print(node->data.when_stmt.condition, indent + 1);
    break;

  case NODE_EXPR_STMT:
    printf("ExprStmt\n");
    ast_print(node->data.expr_stmt.expr, indent + 1);
    break;

  case NODE_BINARY_OP:
    printf("BinaryOp: %s\n", binop_to_str(node->data.binary.op));
    ast_print(node->data.binary.left, indent + 1);
    ast_print(node->data.binary.right, indent + 1);
    break;

  case NODE_UNARY_OP:
    printf("UnaryOp: %s\n", unop_to_str(node->data.unary.op));
    ast_print(node->data.unary.operand, indent + 1);
    break;

  case NODE_WAND_CALL:
    printf("Wand: /%s\n", node->data.wand_call.name);
    if (node->data.wand_call.args) {
      for (size_t i = 0; i < node->data.wand_call.args->count; i++) {
        ast_print(node->data.wand_call.args->items[i], indent + 1);
      }
    }
    break;

  case NODE_IDENTIFIER:
    printf("Identifier: %s\n", node->data.identifier.name);
    break;

  case NODE_INT_LITERAL:
    printf("Int: %d\n", node->data.int_literal.value);
    break;

  case NODE_FLOAT_LITERAL:
    printf("Float: %f\n", node->data.float_literal.value);
    break;

  case NODE_STRING_LITERAL:
    printf("String: \"%s\"\n", node->data.string_literal.value);
    break;

  case NODE_BOOL_LITERAL:
    printf("Bool: %s\n", node->data.bool_literal.value ? "true" : "false");
    break;

  case NODE_ASSIGN:
    printf("Assign\n");
    ast_print(node->data.assign.target, indent + 1);
    ast_print(node->data.assign.value, indent + 1);
    break;

  case NODE_INDEX:
    printf("Index\n");
    ast_print(node->data.index.array, indent + 1);
    ast_print(node->data.index.index, indent + 1);
    break;

  case NODE_IMPLICIT:
    printf("Implicit: _\n");
    break;

  case NODE_ARRAY:
    printf("Array\n");
    if (node->data.array.elements) {
      for (size_t i = 0; i < node->data.array.elements->count; i++) {
        ast_print(node->data.array.elements->items[i], indent + 1);
      }
    }
    break;

  case NODE_OBJECT:
    printf("Object\n");
    if (node->data.object.fields) {
      for (size_t i = 0; i < node->data.object.fields->count; i++) {
        ast_print(node->data.object.fields->items[i], indent + 1);
      }
    }
    break;

  case NODE_OBJECT_FIELD:
    printf("Field: %s\n", node->data.object_field.key);
    ast_print(node->data.object_field.value, indent + 1);
    break;

  case NODE_MEMBER:
    printf("Member: .%s\n", node->data.member.member);
    ast_print(node->data.member.object, indent + 1);
    break;

  case NODE_RANGE:
    printf("Range\n");
    ast_print(node->data.range.start, indent + 1);
    ast_print(node->data.range.end, indent + 1);
    break;

  case NODE_LAMBDA:
    printf("Lambda\n");
    ast_print(node->data.lambda.body, indent + 1);
    break;

  case NODE_MATCH:
    printf("Match\n");
    if (node->data.match.arms) {
      for (size_t i = 0; i < node->data.match.arms->count; i++) {
        ast_print(node->data.match.arms->items[i], indent + 1);
      }
    }
    break;

  case NODE_MATCH_ARM:
    printf("MatchArm\n");
    ast_print(node->data.match_arm.pattern, indent + 1);
    ast_print(node->data.match_arm.body, indent + 1);
    break;

  case NODE_PIPE:
    printf("Pipe\n");
    ast_print(node->data.pipe.left, indent + 1);
    ast_print(node->data.pipe.right, indent + 1);
    break;

  case NODE_TERNARY:
    printf("Ternary\n");
    ast_print(node->data.ternary.condition, indent + 1);
    ast_print(node->data.ternary.then_expr, indent + 1);
    ast_print(node->data.ternary.else_expr, indent + 1);
    break;

  case NODE_USE:
    printf("Use: \"%s\"\n", node->data.use_stmt.path);
    break;
  }
}
