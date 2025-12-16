#include "ast.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yyparse(void);
extern FILE *yyin;
extern ASTNode *ast_root;
extern int yylineno;
extern int yycolumn;

// Forward declaration from codegen
void codegen(ASTNode *root, FILE *output);

// Track included files to prevent circular includes
#define MAX_INCLUDES 256
static char *included_files[MAX_INCLUDES];
static int included_count = 0;

static int is_already_included(const char *path) {
  for (int i = 0; i < included_count; i++) {
    if (strcmp(included_files[i], path) == 0) {
      return 1;
    }
  }
  return 0;
}

static void mark_included(const char *path) {
  if (included_count < MAX_INCLUDES) {
    included_files[included_count++] = strdup(path);
  }
}

// Resolve path relative to base file
static char *resolve_path(const char *base_file, const char *import_path) {
  char *base_copy = strdup(base_file);
  char *dir = dirname(base_copy);

  char *result = malloc(strlen(dir) + strlen(import_path) + 2);
  sprintf(result, "%s/%s", dir, import_path);

  free(base_copy);
  return result;
}

// Parse a file and return its AST (without wrapping in program node)
static ASTNode *parse_file(const char *path);

// Process @use statements recursively
static void process_uses(ASTNode *program, const char *base_file) {
  if (!program || program->type != NODE_PROGRAM)
    return;

  ASTList *decls = program->data.program.decls;
  if (!decls)
    return;

  // Process in reverse to prepend included content at the right position
  for (size_t i = 0; i < decls->count; i++) {
    ASTNode *node = decls->items[i];
    if (node->type == NODE_USE) {
      const char *import_path = node->data.use_stmt.path;
      char *full_path = resolve_path(base_file, import_path);

      if (is_already_included(full_path)) {
        // Already included, skip
        free(full_path);
        continue;
      }

      mark_included(full_path);

      ASTNode *included = parse_file(full_path);
      if (included && included->type == NODE_PROGRAM) {
        // Recursively process uses in the included file
        process_uses(included, full_path);

        // Merge declarations from included file
        ASTList *inc_decls = included->data.program.decls;
        if (inc_decls) {
          // Insert included declarations before current position
          // Create new list with merged content
          ASTList *new_decls = ast_list_new();

          // Add items before @use
          for (size_t j = 0; j < i; j++) {
            ast_list_append(new_decls, decls->items[j]);
          }

          // Add included items (skip USE nodes)
          for (size_t j = 0; j < inc_decls->count; j++) {
            if (inc_decls->items[j]->type != NODE_USE) {
              ast_list_append(new_decls, inc_decls->items[j]);
            }
          }

          // Add items after @use (skip the @use itself)
          for (size_t j = i + 1; j < decls->count; j++) {
            ast_list_append(new_decls, decls->items[j]);
          }

          // Update program with new decls
          size_t old_i = i;
          program->data.program.decls = new_decls;
          decls = new_decls;

          // Adjust index to account for inserted items
          i = old_i + inc_decls->count - 1;
        }
      }

      free(full_path);
    }
  }
}

static ASTNode *parse_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Error: Cannot open file: %s\n", path);
    return NULL;
  }

  // Save and reset parser state
  FILE *old_yyin = yyin;
  ASTNode *old_root = ast_root;
  int old_lineno = yylineno;
  int old_column = yycolumn;

  yyin = f;
  ast_root = NULL;
  yylineno = 1;
  yycolumn = 1;

  int result = yyparse();
  fclose(f);

  ASTNode *parsed = ast_root;

  // Restore parser state
  yyin = old_yyin;
  ast_root = old_root;
  yylineno = old_lineno;
  yycolumn = old_column;

  if (result != 0) {
    fprintf(stderr, "Error parsing: %s\n", path);
    return NULL;
  }

  return parsed;
}

void print_usage(const char *prog) {
  fprintf(stderr, "Usage: %s [options] <input.ds>\n", prog);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -o <file>    Output file (default: stdout)\n");
  fprintf(stderr, "  --ast        Print AST instead of generating code\n");
  fprintf(stderr, "  -h, --help   Show this help\n");
}

int main(int argc, char **argv) {
  const char *input_file = NULL;
  const char *output_file = NULL;
  int print_ast = 0;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      output_file = argv[++i];
    } else if (strcmp(argv[i], "--ast") == 0) {
      print_ast = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (argv[i][0] != '-') {
      input_file = argv[i];
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      print_usage(argv[0]);
      return 1;
    }
  }

  if (!input_file) {
    fprintf(stderr, "Error: No input file specified\n");
    print_usage(argv[0]);
    return 1;
  }

  // Mark main file as included
  mark_included(input_file);

  // Open input file
  yyin = fopen(input_file, "r");
  if (!yyin) {
    fprintf(stderr, "Error: Cannot open input file: %s\n", input_file);
    return 1;
  }

  // Parse
  int parse_result = yyparse();
  fclose(yyin);

  if (parse_result != 0) {
    fprintf(stderr, "Parsing failed\n");
    return 1;
  }

  if (!ast_root) {
    fprintf(stderr, "No AST generated\n");
    return 1;
  }

  // Process @use statements
  process_uses(ast_root, input_file);

  if (print_ast) {
    // Just print the AST
    ast_print(ast_root, 0);
  } else {
    // Generate C code
    FILE *out = stdout;
    if (output_file) {
      out = fopen(output_file, "w");
      if (!out) {
        fprintf(stderr, "Error: Cannot open output file: %s\n", output_file);
        return 1;
      }
    }

    codegen(ast_root, out);

    if (output_file) {
      fclose(out);
    }
  }

  return 0;
}
