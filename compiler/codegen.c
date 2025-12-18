#include "ast.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *out;
static int indent_level = 0;
static int temp_counter = 0;
static int lambda_counter = 0;
static int in_main = 0; // Track if we're generating main() function

// GC root tracking - collect global arrays and string variables
#define MAX_GC_ROOT_ARRAYS 1024
#define MAX_GC_ROOT_VALUES 1024

typedef struct {
  char name[64];
  int is_array; // 1 = array, 0 = single value
} GcRootInfo;

static GcRootInfo gc_root_arrays[MAX_GC_ROOT_ARRAYS];
static int gc_root_array_count = 0;

static GcRootInfo gc_root_values[MAX_GC_ROOT_VALUES];
static int gc_root_value_count = 0;

static void collect_gc_root_array(const char *name) {
  if (gc_root_array_count < MAX_GC_ROOT_ARRAYS) {
    strncpy(gc_root_arrays[gc_root_array_count].name, name, 63);
    gc_root_arrays[gc_root_array_count].name[63] = '\0';
    gc_root_arrays[gc_root_array_count].is_array = 1;
    gc_root_array_count++;
  }
}

static void collect_gc_root_value(const char *name) {
  if (gc_root_value_count < MAX_GC_ROOT_VALUES) {
    strncpy(gc_root_values[gc_root_value_count].name, name, 63);
    gc_root_values[gc_root_value_count].name[63] = '\0';
    gc_root_values[gc_root_value_count].is_array = 0;
    gc_root_value_count++;
  }
}

// Track context for implicit _ (the piped/matched value)
#define MAX_CONTEXT_DEPTH 32
static const char *implicit_context[MAX_CONTEXT_DEPTH];
static int implicit_depth = 0;

static void push_implicit(const char *name) {
  if (implicit_depth < MAX_CONTEXT_DEPTH) {
    implicit_context[implicit_depth++] = name;
  }
}

static void pop_implicit(void) {
  if (implicit_depth > 0) {
    implicit_depth--;
  }
}

static const char *get_implicit(void) {
  if (implicit_depth > 0) {
    return implicit_context[implicit_depth - 1];
  }
  return "__implicit_undefined";
}

// C reserved words that might conflict with DungeonScript function names
static const char *c_reserved[] = {
    "auto",     "break",  "case",    "char",     "const",      "continue",
    "default",  "do",     "double",  "else",     "enum",       "extern",
    "float",    "for",    "goto",    "if",       "int",        "long",
    "register", "return", "short",   "signed",   "sizeof",     "static",
    "struct",   "switch", "typedef", "union",    "unsigned",   "void",
    "volatile", "while",  "_Bool",   "_Complex", "_Imaginary", "inline",
    "restrict", NULL};

static int is_c_reserved(const char *name) {
  for (int i = 0; c_reserved[i] != NULL; i++) {
    if (strcmp(name, c_reserved[i]) == 0)
      return 1;
  }
  return 0;
}

// Buffer for mangled names (static, reused)
static char mangled_name_buf[256];

// Mangle function name to avoid C conflicts - prefix with ds_
static const char *mangle_func_name(const char *name) {
  if (is_c_reserved(name)) {
    snprintf(mangled_name_buf, sizeof(mangled_name_buf), "ds_%s", name);
    return mangled_name_buf;
  }
  return name;
}

static void emit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  for (int i = 0; i < indent_level; i++)
    fprintf(out, "    ");
  vfprintf(out, fmt, args);
  va_end(args);
}

static void emit_raw(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(out, fmt, args);
  va_end(args);
}

static const char *binop_to_c(BinaryOp op) {
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
    return "&&";
  case OP_OR:
    return "||";
  default:
    return "?";
  }
}

// Forward declarations
static void codegen_expr(ASTNode *node);
static void codegen_stmt(ASTNode *node);

// Identify which function arguments should be treated as floats
static int is_float_func_arg(const char *name, int arg_idx) {
  if (strcmp(name, "gl_clear_color") == 0)
    return 1; // All 4 args
  if (strcmp(name, "gl_uniform1f") == 0 && arg_idx >= 1)
    return 1;
  if (strcmp(name, "gl_uniform2f") == 0 && arg_idx >= 1)
    return 1;
  if (strcmp(name, "gl_uniform3f") == 0 && arg_idx >= 1)
    return 1;
  if (strcmp(name, "gl_uniform4f") == 0 && arg_idx >= 1)
    return 1;
  if (strcmp(name, "buf_set_float") == 0 && arg_idx == 2)
    return 1;
  return 0;
}

// Registry of variables known to hold float values
#define MAX_FLOAT_VARS 256
static char *float_vars[MAX_FLOAT_VARS];
static int float_var_count = 0;

static void register_float_var(const char *name) {
  if (float_var_count < MAX_FLOAT_VARS) {
    float_vars[float_var_count++] = strdup(name);
  }
}

static int is_float_var(const char *name) {
  for (int i = 0; i < float_var_count; i++) {
    if (strcmp(float_vars[i], name) == 0)
      return 1;
  }
  return 0;
}

// Helper to check if an expression involves floats (recursively)
static int is_float_expr(ASTNode *node) {
  if (!node)
    return 0;
  switch (node->type) {
  case NODE_FLOAT_LITERAL:
    return 1;
  case NODE_IDENTIFIER:
    // Check if this variable was declared with a float expression
    return is_float_var(node->data.identifier.name);
  case NODE_BINARY_OP:
    // If either operand involves floats, the result is float
    return is_float_expr(node->data.binary.left) ||
           is_float_expr(node->data.binary.right);
  case NODE_UNARY_OP:
    return is_float_expr(node->data.unary.operand);
  default:
    return 0;
  }
}

// Emit an expression for use in a float context
// Non-float identifiers (function params, etc.) are tagged integers - unwrap
// them
static void codegen_float_operand(ASTNode *node) {
  if (node && node->type == NODE_IDENTIFIER &&
      !is_float_var(node->data.identifier.name)) {
    // This is a non-float identifier (likely a function parameter) - unwrap it
    emit_raw("AS_INT(%s)", node->data.identifier.name);
  } else {
    codegen_expr(node);
  }
}

// Structure to collect lambdas for forward declaration
typedef struct {
  int id;
  ASTList *params;
  ASTNode *body;
} LambdaInfo;

#define MAX_LAMBDAS 256
static LambdaInfo collected_lambdas[MAX_LAMBDAS];
static int collected_lambda_count = 0;

// Collect lambda for later emission
static int collect_lambda(ASTList *params, ASTNode *body) {
  int id = lambda_counter++;
  if (collected_lambda_count < MAX_LAMBDAS) {
    collected_lambdas[collected_lambda_count].id = id;
    collected_lambdas[collected_lambda_count].params = params;
    collected_lambdas[collected_lambda_count].body = body;
    collected_lambda_count++;
  }
  return id;
}

static void codegen_expr(ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case NODE_INT_LITERAL:
    emit_raw("VAL_INT(%ld)", (long)node->data.int_literal.value);
    break;

  case NODE_FLOAT_LITERAL:
    emit_raw("%f", node->data.float_literal.value);
    break;

  case NODE_STRING_LITERAL: {
    // Escape special characters for C string literal
    // Cast to Value (OBJ) so it can be passed to runtime functions
    const char *s = node->data.string_literal.value;
    emit_raw("VAL_OBJ(\"");
    for (; *s; s++) {
      switch (*s) {
      case '\n':
        emit_raw("\\n");
        break;
      case '\r':
        emit_raw("\\r");
        break;
      case '\t':
        emit_raw("\\t");
        break;
      case '\\':
        emit_raw("\\\\");
        break;
      case '"':
        emit_raw("\\\"");
        break;
      default:
        emit_raw("%c", *s);
        break;
      }
    }
    emit_raw("\")");
    break;
  }

  case NODE_BOOL_LITERAL:
    emit_raw("VAL_INT(%ld)", (long)node->data.bool_literal.value);
    break;

  case NODE_IDENTIFIER:
    emit_raw("%s", node->data.identifier.name);
    break;

  case NODE_IMPLICIT:
    // Use the current implicit context
    emit_raw("%s", get_implicit());
    break;

  case NODE_BINARY_OP:
    // Special handling for comparisons - result is boolean (INT 0 or 1)
    if (node->data.binary.op >= OP_EQ && node->data.binary.op <= OP_OR) {
      // Logical/Comparison ops return tagged booleans
      // Operands for logical ops (AND/OR) are treated as booleans (check != 0)
      // Operands for comparison ops (EQ/NE/LT...) need AS_INT unless we do
      // operator overloading For simplicity: Int comparisons
      if (node->data.binary.op == OP_EQ) {
        emit_raw("val_eq((");
        codegen_expr(node->data.binary.left);
        emit_raw("), (");
        codegen_expr(node->data.binary.right);
        emit_raw("))");
      } else if (node->data.binary.op == OP_NE) {
        emit_raw("(val_eq((");
        codegen_expr(node->data.binary.left);
        emit_raw("), (");
        codegen_expr(node->data.binary.right);
        emit_raw(")) == VAL_INT(0) ? VAL_INT(1) : VAL_INT(0))");
      } else if (node->data.binary.op == OP_AND) {
        emit_raw("(((");
        codegen_expr(node->data.binary.left);
        emit_raw(") != VAL_INT(0)) && ((");
        codegen_expr(node->data.binary.right);
        emit_raw(") != VAL_INT(0)) ? VAL_INT(1) : VAL_INT(0))");
      } else if (node->data.binary.op == OP_OR) {
        emit_raw("(((");
        codegen_expr(node->data.binary.left);
        emit_raw(") != VAL_INT(0)) || ((");
        codegen_expr(node->data.binary.right);
        emit_raw(") != VAL_INT(0)) ? VAL_INT(1) : VAL_INT(0))");
      } else {
        // Relational ops ( <, >, <=, >= ) assume integers for now
        emit_raw("VAL_INT(AS_INT(");
        codegen_expr(node->data.binary.left);
        emit_raw(") %s AS_INT(", binop_to_c(node->data.binary.op));
        codegen_expr(node->data.binary.right);
        emit_raw("))");
      }
    } else {
      // Arithmetic ops ( +, -, *, /, % )
      // Check if either operand is a float - if so, emit raw operation
      if (is_float_expr(node->data.binary.left) ||
          is_float_expr(node->data.binary.right)) {
        emit_raw("(");
        codegen_float_operand(node->data.binary.left);
        emit_raw(" %s ", binop_to_c(node->data.binary.op));
        codegen_float_operand(node->data.binary.right);
        emit_raw(")");
      } else {
        emit_raw("VAL_INT(AS_INT(");
        codegen_expr(node->data.binary.left);
        emit_raw(") %s AS_INT(", binop_to_c(node->data.binary.op));
        codegen_expr(node->data.binary.right);
        emit_raw("))");
      }
    }
    break;

  case NODE_UNARY_OP:
    switch (node->data.unary.op) {
    case OP_NEG:
      if (is_float_expr(node->data.unary.operand)) {
        emit_raw("(-");
        codegen_expr(node->data.unary.operand);
        emit_raw(")");
      } else {
        emit_raw("VAL_INT(-AS_INT(");
        codegen_expr(node->data.unary.operand);
        emit_raw("))");
      }
      break;
    case OP_NOT:
      emit_raw("VAL_INT((");
      codegen_expr(node->data.unary.operand);
      emit_raw(") == VAL_INT(0))");
      break;
    }
    break;

  case NODE_WAND_CALL: {
    const char *fname = mangle_func_name(node->data.wand_call.name);
    emit_raw("%s(", fname);
    if (node->data.wand_call.args) {
      for (size_t i = 0; i < node->data.wand_call.args->count; i++) {
        if (i > 0)
          emit_raw(", ");
        ASTNode *arg = node->data.wand_call.args->items[i];
        if (is_float_func_arg(node->data.wand_call.name, (int)i)) {
          // Arg expects float: if expr is float, emit raw, else untag int
          if (is_float_expr(arg)) {
            codegen_expr(arg);
          } else {
            emit_raw("AS_INT(");
            codegen_expr(arg);
            emit_raw(")");
          }
        } else {
          codegen_expr(arg);
        }
      }
    }
    emit_raw(")");
    break;
  }

  case NODE_ASSIGN:
    // Check if assigning to a member
    if (node->data.assign.target->type == NODE_MEMBER) {
      emit_raw("ds_object_set(&");
      codegen_expr(node->data.assign.target->data.member.object);
      emit_raw(", VAL_OBJ(\"%s\"), ",
               node->data.assign.target->data.member.member);
      codegen_expr(node->data.assign.value);
      emit_raw(")");
    } else {
      codegen_expr(node->data.assign.target);
      emit_raw(" = ");
      codegen_expr(node->data.assign.value);
    }
    break;

  case NODE_INDEX:
    codegen_expr(node->data.index.array);
    emit_raw("[AS_INT(");
    codegen_expr(node->data.index.index);
    emit_raw(")]");
    break;

  case NODE_ARRAY:
    // Array literals generate brace initializers
    // Note: This is only valid in declaration context (VAR_DECL handles this)
    emit_raw("{");
    if (node->data.array.elements && node->data.array.elements->count > 0) {
      for (size_t i = 0; i < node->data.array.elements->count; i++) {
        if (i > 0)
          emit_raw(", ");
        codegen_expr(node->data.array.elements->items[i]);
      }
    } else {
      emit_raw("0"); // Empty array initializer
    }
    emit_raw("}");
    break;

  case NODE_RANGE:
    // Ranges are handled specially in for loops
    emit_raw("/* range */");
    break;

  case NODE_TERNARY:
    emit_raw("((");
    codegen_expr(node->data.ternary.condition);
    emit_raw(") != VAL_INT(0) ? ");
    codegen_expr(node->data.ternary.then_expr);
    emit_raw(" : ");
    codegen_expr(node->data.ternary.else_expr);
    emit_raw(")");
    break;

  case NODE_PIPE:
    // Pipe: left | right
    // If right is a match expression, generate pattern matching
    if (node->data.pipe.right->type == NODE_MATCH) {
      // Generate GCC statement expression for pattern matching
      // ({ long __match_N = left; long __result_N; if (...) ... __result_N; })
      int match_id = temp_counter++;
      emit_raw("({ long __match_%d = ", match_id);
      codegen_expr(node->data.pipe.left);
      emit_raw("; long __result_%d; ", match_id);

      // Push implicit context for _ in match arms
      char implicit_name[32];
      snprintf(implicit_name, sizeof(implicit_name), "__match_%d", match_id);
      push_implicit(implicit_name);

      // Generate if-else chain for match arms
      ASTList *arms = node->data.pipe.right->data.match.arms;
      if (arms) {
        for (size_t i = 0; i < arms->count; i++) {
          ASTNode *arm = arms->items[i];
          ASTNode *pattern = arm->data.match_arm.pattern;
          ASTNode *body = arm->data.match_arm.body;

          if (i > 0)
            emit_raw(" else ");

          // Check if pattern is wildcard (_)
          if (pattern->type == NODE_IMPLICIT) {
            // Default case - just execute
            emit_raw("{ __result_%d = ", match_id);
            codegen_expr(body);
            emit_raw("; }");
          } else {
            // Compare pattern to match value
            emit_raw("if (__match_%d == ", match_id);
            codegen_expr(pattern);
            emit_raw(") { __result_%d = ", match_id);
            codegen_expr(body);
            emit_raw("; }");
          }
        }
      }

      pop_implicit();
      emit_raw(" __result_%d; })", match_id);
    }
    // If right is a wand call, pipe left as first argument
    else if (node->data.pipe.right->type == NODE_WAND_CALL) {
      const char *fname =
          mangle_func_name(node->data.pipe.right->data.wand_call.name);
      emit_raw("%s(", fname);
      codegen_expr(node->data.pipe.left);
      if (node->data.pipe.right->data.wand_call.args) {
        for (size_t i = 0;
             i < node->data.pipe.right->data.wand_call.args->count; i++) {
          emit_raw(", ");
          codegen_expr(node->data.pipe.right->data.wand_call.args->items[i]);
        }
      }
      emit_raw(")");
    }
    // If right is a lambda, apply it to left
    else if (node->data.pipe.right->type == NODE_LAMBDA) {
      // Inline the lambda: ({ long param = left; body; })
      ASTNode *lambda = node->data.pipe.right;
      int lambda_id = temp_counter++;
      emit_raw("({ ");

      // Declare parameters with the piped value
      if (lambda->data.lambda.params && lambda->data.lambda.params->count > 0) {
        ASTNode *param = lambda->data.lambda.params->items[0];
        emit_raw("long %s = ", param->data.param.name);
        codegen_expr(node->data.pipe.left);
        emit_raw("; ");
        push_implicit(param->data.param.name);
      } else {
        // No params, use implicit
        char implicit_name[32];
        snprintf(implicit_name, sizeof(implicit_name), "__pipe_%d", lambda_id);
        emit_raw("long %s = ", implicit_name);
        codegen_expr(node->data.pipe.left);
        emit_raw("; ");
        push_implicit(implicit_name);
      }

      // Generate lambda body
      if (lambda->data.lambda.body->type == NODE_BLOCK) {
        // Block body - need to track result
        emit_raw("/* block lambda not fully supported */ ");
        codegen_expr(node->data.pipe.left);
      } else {
        // Expression body
        codegen_expr(lambda->data.lambda.body);
      }

      pop_implicit();
      emit_raw("; })");
    }
    // If right is an identifier, call it as a function with left as arg
    else if (node->data.pipe.right->type == NODE_IDENTIFIER) {
      emit_raw("%s(", node->data.pipe.right->data.identifier.name);
      codegen_expr(node->data.pipe.left);
      emit_raw(")");
    } else {
      // Generic: evaluate right with implicit set to left
      int pipe_id = temp_counter++;
      char implicit_name[32];
      snprintf(implicit_name, sizeof(implicit_name), "__pipe_%d", pipe_id);
      emit_raw("({ long %s = ", implicit_name);
      codegen_expr(node->data.pipe.left);
      emit_raw("; ");
      push_implicit(implicit_name);
      codegen_expr(node->data.pipe.right);
      pop_implicit();
      emit_raw("; })");
    }
    break;

  case NODE_MATCH:
    // Standalone match without pipe - shouldn't happen in normal use
    // but handle it by returning 0
    emit_raw("0 /* standalone match */");
    break;

  case NODE_LAMBDA: {
    // Lambda as a value - collect it and emit reference
    // For now, lambdas are only useful when directly piped
    // We'll generate a static function and return a function pointer
    int id = collect_lambda(node->data.lambda.params, node->data.lambda.body);
    emit_raw("__lambda_%d", id);
    break;
  } break;

  case NODE_OBJECT:
    // Objects are implemented as a simple struct with string keys
    // We'll generate a compound literal with the ds_object type
    emit_raw("ds_object_create(VAL_INT(%zu)",
             node->data.object.fields ? node->data.object.fields->count : 0);
    if (node->data.object.fields) {
      for (size_t i = 0; i < node->data.object.fields->count; i++) {
        ASTNode *field = node->data.object.fields->items[i];
        emit_raw(", \"%s\", ", field->data.object_field.key);
        codegen_expr(field->data.object_field.value);
      }
    }
    emit_raw(")");
    break;

  case NODE_OBJECT_FIELD:
    // Shouldn't be reached directly - handled by NODE_OBJECT
    emit_raw("/* field */");
    break;

  case NODE_MEMBER:
    // Member access: obj.field becomes ds_object_get(obj, (Value)"field")
    emit_raw("ds_object_get(");
    codegen_expr(node->data.member.object);
    emit_raw(", VAL_OBJ(\"%s\"))", node->data.member.member);
    break;

  default:
    emit_raw("/* unknown expr */0");
    break;
  }
}

static void codegen_stmt(ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case NODE_BLOCK:
    emit("{\n");
    indent_level++;
    if (node->data.block.statements) {
      for (size_t i = 0; i < node->data.block.statements->count; i++) {
        codegen_stmt(node->data.block.statements->items[i]);
      }
    }
    indent_level--;
    emit("}\n");
    break;

  case NODE_VAR_DECL:
    // Detect type from initializer
    if (node->data.var_decl.init &&
        node->data.var_decl.init->type == NODE_ARRAY) {
      emit("long %s[16384] = ", node->data.var_decl.name);
      codegen_expr(node->data.var_decl.init);
      emit_raw(";\n");
    } else if (node->data.var_decl.init &&
               is_float_expr(node->data.var_decl.init)) {
      // Variable holds a float value - emit double and register
      register_float_var(node->data.var_decl.name);
      emit("double %s = ", node->data.var_decl.name);
      codegen_expr(node->data.var_decl.init);
      emit_raw(";\n");
    } else if (node->data.var_decl.init &&
               node->data.var_decl.init->type == NODE_STRING_LITERAL) {
      // Strings are stored as Value (pointer cast to long) for consistency
      emit("Value %s = ", node->data.var_decl.name);
      codegen_expr(
          node->data.var_decl.init); // codegen_expr now emits VAL_OBJ("...")
      emit_raw(";\n");
    } else if (node->data.var_decl.init &&
               node->data.var_decl.init->type == NODE_OBJECT) {
      emit("long %s = ", node->data.var_decl.name);
      codegen_expr(node->data.var_decl.init);
      emit_raw(";\n");
    } else {
      emit("long %s = ", node->data.var_decl.name);
      codegen_expr(node->data.var_decl.init);
      emit_raw(";\n");
    }
    break;

  case NODE_ASSIGN:
    emit("");
    // Check if assigning to a member
    if (node->data.assign.target->type == NODE_MEMBER) {
      emit_raw("ds_object_set(&");
      codegen_expr(node->data.assign.target->data.member.object);
      emit_raw(", VAL_OBJ(\"%s\"), ",
               node->data.assign.target->data.member.member);
      codegen_expr(node->data.assign.value);
      emit_raw(");\n");
    } else {
      codegen_expr(node->data.assign.target);
      emit_raw(" = ");
      codegen_expr(node->data.assign.value);
      emit_raw(";\n");
    }
    break;

  case NODE_LOOP:
    if (node->data.loop.condition) {
      emit("while ((");
      codegen_expr(node->data.loop.condition);
      emit_raw(") != VAL_INT(0)) ");
    } else {
      emit("while (1) ");
    }
    codegen_stmt(node->data.loop.body);
    break;

  case NODE_FOR:
    // Check if iterating over a range
    if (node->data.for_loop.iterable->type == NODE_RANGE) {
      ASTNode *range = node->data.for_loop.iterable;
      emit("for (long %s = ", node->data.for_loop.var_name);
      codegen_expr(range->data.range.start);
      emit_raw("; %s < ", node->data.for_loop.var_name);
      codegen_expr(
          range->data.range.end); // Range end is already tagged int? No, range
                                  // loop var usually raw int in C for loop??
      // Wait, if we use tagged ints for loop variable, ++ works differently
      // (+2). If we use raw C long for loop var, we must tag it when using it
      // in body? Currently codegen emits `long %s`. If `%s` is used in body, it
      // is an IDENTIFIER. codegen_expr emits name. If name is raw long, using
      // it in expression expects tagged. FIX: Range loop variable should be raw
      // long for loop mechanics, but wrapped when accessed? Or we make the loop
      // variable a tagged int and increment by 2? "long x = VAL_INT(0); x <
      // VAL_INT(10); x += 2" AST_NEW_RANGE takes start/end nodes. Let's assume
      // loop var is TAGGED. Start/End are likely integer literals ->
      // VAL_INT(...). So `for (long i = start; i < end; i += 2)` (if < compares
      // tagged correctly?) VAL_INT(a) < VAL_INT(b) is same as a < b iff
      // ((a<<1)|1) < ((b<<1)|1). Yes. So we can loop on tagged values!
      // Increment must be specialized? Generic `i++` adds 1. Tagged logic: `i =
      // VAL_INT(AS_INT(i) + 1)` -> `i += 2`. But `i++` in C adds 1. For now,
      // let's keep it simple: Use tagged values in loop. Change `i++` to `i = i
      // + 2` ?? No `++` is concise. Actually `+= 2` is fine. `start` and `end`
      // are expressions.
      emit_raw("; %s += 2) ", node->data.for_loop.var_name);
      codegen_stmt(node->data.for_loop.body);
    } else {
      // Generic iteration - need runtime support
      emit("/* for %s in ... */ ", node->data.for_loop.var_name);
      codegen_stmt(node->data.for_loop.body);
    }
    break;

  case NODE_RETURN:
    if (in_main) {
      // main() should return raw int for proper exit code
      emit("return AS_INT(");
      if (node->data.return_stmt.value) {
        codegen_expr(node->data.return_stmt.value);
      } else {
        emit_raw("0");
      }
      emit_raw(");\n");
    } else {
      emit("return");
      if (node->data.return_stmt.value) {
        emit_raw(" ");
        codegen_expr(node->data.return_stmt.value);
      }
      emit_raw(";\n");
    }
    break;

  case NODE_BREAK:
    if (node->data.break_stmt.condition) {
      emit("if ((");
      codegen_expr(node->data.break_stmt.condition);
      emit_raw(") != VAL_INT(0)) break;\n");
    } else {
      emit("break;\n");
    }
    break;

  case NODE_CONTINUE:
    emit("continue;\n");
    break;

  case NODE_WHEN_STMT:
    if (node->data.when_stmt.is_unless) {
      emit("if ((");
      codegen_expr(node->data.when_stmt.condition);
      emit_raw(") == VAL_INT(0)) {\n");
    } else {
      emit("if ((");
      codegen_expr(node->data.when_stmt.condition);
      emit_raw(") != VAL_INT(0)) {\n");
    }
    indent_level++;
    // Inline the action
    if (node->data.when_stmt.action->type == NODE_BLOCK) {
      // Block body - generate all statements
      if (node->data.when_stmt.action->data.block.statements) {
        for (size_t i = 0; i < node->data.when_stmt.action->data.block.statements->count; i++) {
          codegen_stmt(node->data.when_stmt.action->data.block.statements->items[i]);
        }
      }
    } else if (node->data.when_stmt.action->type == NODE_WAND_CALL) {
      emit("");
      codegen_expr(node->data.when_stmt.action);
      emit_raw(";\n");
    } else if (node->data.when_stmt.action->type == NODE_BREAK) {
      emit("break;\n");
    } else if (node->data.when_stmt.action->type == NODE_CONTINUE) {
      emit("continue;\n");
    } else if (node->data.when_stmt.action->type == NODE_RETURN) {
      emit("return");
      if (node->data.when_stmt.action->data.return_stmt.value) {
        emit_raw(" ");
        codegen_expr(node->data.when_stmt.action->data.return_stmt.value);
      }
      emit_raw(";\n");
    } else {
      emit("");
      codegen_expr(node->data.when_stmt.action);
      emit_raw(";\n");
    }
    indent_level--;
    emit("}\n");
    break;

  case NODE_EXPR_STMT:
    emit("");
    codegen_expr(node->data.expr_stmt.expr);
    emit_raw(";\n");
    break;

  default:
    emit("/* unknown stmt */\n");
    break;
  }
}

static int function_has_return(ASTNode *body) {
  if (!body)
    return 0;
  if (body->type == NODE_RETURN && body->data.return_stmt.value)
    return 1;
  if (body->type == NODE_WHEN_STMT) {
    if (body->data.when_stmt.action->type == NODE_RETURN &&
        body->data.when_stmt.action->data.return_stmt.value)
      return 1;
  }
  if (body->type == NODE_BLOCK && body->data.block.statements) {
    for (size_t i = 0; i < body->data.block.statements->count; i++) {
      if (function_has_return(body->data.block.statements->items[i]))
        return 1;
    }
  }
  return 0;
}

static void codegen_function(ASTNode *func) {
  const char *name = func->data.function.name;
  int is_main = (strcmp(name, "main") == 0);
  int is_game_init = (strcmp(name, "game_init") == 0);
  in_main = is_main;

  const char *ret_type =
      is_main
          ? "int"
          : (function_has_return(func->data.function.body) ? "long" : "void");
  const char *mangled_name = mangle_func_name(name);
  emit("%s %s(", ret_type, mangled_name);

  ASTList *params = func->data.function.params;
  if (params && params->count > 0) {
    for (size_t i = 0; i < params->count; i++) {
      if (i > 0)
        emit_raw(", ");
      ASTNode *p = params->items[i];
      emit_raw("long %s", p->data.param.name);
    }
  } else {
    emit_raw("void");
  }

  emit_raw(") ");

  if (func->data.function.body->type == NODE_RETURN) {
    emit_raw("{\n");
    indent_level++;
    // Inject GC root registration at start of game_init
    if (is_game_init) {
      emit("__gc_register_roots();\n");
    }
    codegen_stmt(func->data.function.body);
    indent_level--;
    emit("}\n");
  } else {
    // For block bodies, inject after opening brace
    if (is_game_init && func->data.function.body->type == NODE_BLOCK) {
      emit("{\n");
      indent_level++;
      emit("__gc_register_roots();\n");
      // Emit block contents
      if (func->data.function.body->data.block.statements) {
        for (size_t i = 0;
             i < func->data.function.body->data.block.statements->count; i++) {
          codegen_stmt(
              func->data.function.body->data.block.statements->items[i]);
        }
      }
      indent_level--;
      emit("}\n");
    } else {
      codegen_stmt(func->data.function.body);
    }
  }
  emit_raw("\n");
}

static void codegen_global_var(ASTNode *node) {
  if (node->data.var_decl.init &&
      node->data.var_decl.init->type == NODE_ARRAY) {
    emit("long %s[16384] = ", node->data.var_decl.name);
    codegen_expr(node->data.var_decl.init);
    emit_raw(";\n");
    // Collect as GC root array
    collect_gc_root_array(node->data.var_decl.name);
  } else if (node->data.var_decl.init &&
             node->data.var_decl.init->type == NODE_FLOAT_LITERAL) {
    emit("double %s = ", node->data.var_decl.name);
    codegen_expr(node->data.var_decl.init);
    emit_raw(";\n");
  } else if (node->data.var_decl.init &&
             node->data.var_decl.init->type == NODE_STRING_LITERAL) {
    // Strings are stored as Value (pointer cast to long) for consistency
    emit("Value %s = ", node->data.var_decl.name);
    codegen_expr(node->data.var_decl.init);
    emit_raw(";\n");
    // Collect as GC root value (may hold dynamically allocated strings later)
    collect_gc_root_value(node->data.var_decl.name);
  } else if (node->data.var_decl.init &&
             node->data.var_decl.init->type == NODE_OBJECT) {
    emit("long %s = ", node->data.var_decl.name);
    codegen_expr(node->data.var_decl.init);
    emit_raw(";\n");
    collect_gc_root_value(node->data.var_decl.name);
  } else {
    emit("long %s = ", node->data.var_decl.name);
    codegen_expr(node->data.var_decl.init);
    emit_raw(";\n");
    collect_gc_root_value(node->data.var_decl.name);
  }
}

// Emit collected lambdas as static functions
static void emit_lambdas(void) {
  for (int i = 0; i < collected_lambda_count; i++) {
    LambdaInfo *lambda = &collected_lambdas[i];

    emit("static long __lambda_%d(", lambda->id);

    if (lambda->params && lambda->params->count > 0) {
      for (size_t j = 0; j < lambda->params->count; j++) {
        if (j > 0)
          emit_raw(", ");
        ASTNode *p = lambda->params->items[j];
        emit_raw("long %s", p->data.param.name);
      }
      // Set first param as implicit context
      push_implicit(lambda->params->items[0]->data.param.name);
    } else {
      emit_raw("long _");
      push_implicit("_");
    }

    emit_raw(") {\n");
    indent_level++;

    if (lambda->body->type == NODE_BLOCK) {
      // Block body
      if (lambda->body->data.block.statements) {
        for (size_t j = 0; j < lambda->body->data.block.statements->count;
             j++) {
          codegen_stmt(lambda->body->data.block.statements->items[j]);
        }
      }
    } else {
      // Expression body
      emit("return ");
      codegen_expr(lambda->body);
      emit_raw(";\n");
    }

    indent_level--;
    emit("}\n\n");
    pop_implicit();
  }
}

// Emit forward declaration for a function
static void codegen_function_decl(ASTNode *func) {
  const char *name = func->data.function.name;
  int is_main = (strcmp(name, "main") == 0);
  const char *ret_type =
      is_main
          ? "int"
          : (function_has_return(func->data.function.body) ? "long" : "void");
  const char *mangled_name = mangle_func_name(name);
  emit("%s %s(", ret_type, mangled_name);

  ASTList *params = func->data.function.params;
  if (params && params->count > 0) {
    for (size_t i = 0; i < params->count; i++) {
      if (i > 0)
        emit_raw(", ");
      ASTNode *p = params->items[i];
      emit_raw("long %s", p->data.param.name);
    }
  } else {
    emit_raw("void");
  }

  emit_raw(");\n");
}

void codegen(ASTNode *root, FILE *output) {
  out = output;
  indent_level = 0;
  temp_counter = 0;
  lambda_counter = 0;
  collected_lambda_count = 0;
  implicit_depth = 0;

  // Reset GC root tracking
  gc_root_array_count = 0;
  gc_root_value_count = 0;

  // Emit header
  fprintf(out, "// Generated by nh compiler\n");
  fprintf(out, "#include \"runtime.h\"\n\n");

  if (root && root->type == NODE_PROGRAM && root->data.program.decls) {
    ASTList *decls = root->data.program.decls;

    // First pass: emit global variables (this collects GC roots)
    for (size_t i = 0; i < decls->count; i++) {
      if (decls->items[i]->type == NODE_VAR_DECL) {
        codegen_global_var(decls->items[i]);
      }
    }
    fprintf(out, "\n");

    // Emit GC root registration function
    fprintf(out, "// GC root registration (auto-generated)\n");
    fprintf(out, "static void __gc_register_roots(void) {\n");
    for (int i = 0; i < gc_root_array_count; i++) {
      fprintf(out, "    gc_register_root_array(%s, 16384);\n",
              gc_root_arrays[i].name);
    }
    for (int i = 0; i < gc_root_value_count; i++) {
      fprintf(out, "    gc_register_root_value(&%s);\n",
              gc_root_values[i].name);
    }
    fprintf(out, "}\n\n");

    // Second pass: emit forward declarations for all functions
    fprintf(out, "// Forward declarations\n");
    for (size_t i = 0; i < decls->count; i++) {
      if (decls->items[i]->type == NODE_FUNCTION) {
        codegen_function_decl(decls->items[i]);
      }
    }
    fprintf(out, "\n");

    // Third pass: emit function definitions
    for (size_t i = 0; i < decls->count; i++) {
      if (decls->items[i]->type == NODE_FUNCTION) {
        codegen_function(decls->items[i]);
      }
    }

    // Emit any collected lambdas
    if (collected_lambda_count > 0) {
      fprintf(out, "// Lambda functions\n");
      emit_lambdas();
    }
  }
}
