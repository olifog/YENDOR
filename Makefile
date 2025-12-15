# nh Language Build System
# Requires: bison, flex, emscripten (emcc)

# Directories
COMPILER_DIR = compiler
RUNTIME_DIR = runtime
GAME_DIR = game
WEB_DIR = web
BUILD_DIR = build
EXAMPLES_DIR = examples

# Tools
BISON = bison
FLEX = flex
CC = gcc
EMCC = emcc

# Flags
CFLAGS = -Wall -Wextra -g
EMFLAGS = -O2 -s WASM=1 \
          -s USE_WEBGL2=1 \
          -s FULL_ES3=1 \
          -s MIN_WEBGL_VERSION=2 \
          -s MAX_WEBGL_VERSION=2 \
          -s OFFSCREENCANVAS_SUPPORT=0 \
          -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
          -s EXPORTED_FUNCTIONS='["_game_init","_game_update","_game_render","_on_key_down","_on_key_up","_on_frame_start","_malloc","_free"]' \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s MODULARIZE=1 -s EXPORT_NAME='createModule' \
          -lm

# Default target
.PHONY: all
all: compiler

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ============================================================================
# Compiler Build (native, runs on host machine)
# ============================================================================

# Generate parser from Bison
$(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/parser.tab.h: $(COMPILER_DIR)/parser.y
	cd $(COMPILER_DIR) && $(BISON) -d parser.y

# Generate lexer from Flex
$(COMPILER_DIR)/lex.yy.c: $(COMPILER_DIR)/lexer.l $(COMPILER_DIR)/parser.tab.h
	cd $(COMPILER_DIR) && $(FLEX) lexer.l

# Build the compiler
.PHONY: compiler
compiler: $(BUILD_DIR) $(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/lex.yy.c
	$(CC) $(CFLAGS) -I$(COMPILER_DIR) \
		$(COMPILER_DIR)/parser.tab.c \
		$(COMPILER_DIR)/lex.yy.c \
		$(COMPILER_DIR)/ast.c \
		$(COMPILER_DIR)/codegen.c \
		$(COMPILER_DIR)/main.c \
		-o $(BUILD_DIR)/dsc

# ============================================================================
# Test the compiler
# ============================================================================

.PHONY: test-compiler
test-compiler: compiler
	@echo "=== Parsing example ===" 
	$(BUILD_DIR)/dsc --ast $(EXAMPLES_DIR)/hello.nh
	@echo ""
	@echo "=== Generating C code ==="
	$(BUILD_DIR)/dsc $(EXAMPLES_DIR)/hello.nh

# ============================================================================
# Compile game to C (using our compiler)
# ============================================================================

$(BUILD_DIR)/game.c: compiler $(GAME_DIR)/main.nh
	$(BUILD_DIR)/dsc $(GAME_DIR)/main.nh -o $(BUILD_DIR)/game.c

# ============================================================================
# Build WASM (using Emscripten)
# ============================================================================

.PHONY: wasm
wasm: $(BUILD_DIR)/game.c
	$(EMCC) $(EMFLAGS) \
		$(BUILD_DIR)/game.c \
		$(RUNTIME_DIR)/runtime.c \
		-I$(RUNTIME_DIR) \
		-o $(BUILD_DIR)/game.js
	cp $(BUILD_DIR)/game.js $(BUILD_DIR)/game.wasm $(WEB_DIR)/

# ============================================================================
# Development server
# ============================================================================

.PHONY: serve
serve:
	@echo "Starting dev server at http://localhost:8080"
	@cd $(WEB_DIR) && python3 -m http.server 8080

.PHONY: serve-build
serve-build: wasm
	@cp $(BUILD_DIR)/game.js $(BUILD_DIR)/game.wasm $(WEB_DIR)/
	@echo "Starting dev server at http://localhost:8080"
	@cd $(WEB_DIR) && python3 -m http.server 8080

# ============================================================================
# Demo (just the web frontend without WASM)
# ============================================================================

.PHONY: demo
demo:
	@echo "Opening demo in browser..."
	@open $(WEB_DIR)/index.html || xdg-open $(WEB_DIR)/index.html || echo "Open $(WEB_DIR)/index.html in your browser"

# ============================================================================
# Clean
# ============================================================================

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/parser.tab.h
	rm -f $(COMPILER_DIR)/lex.yy.c
	rm -f $(WEB_DIR)/game.js $(WEB_DIR)/game.wasm

# ============================================================================
# Help
# ============================================================================

# ============================================================================
# Test Suite
# ============================================================================

.PHONY: test
test: compiler
	@./tests/run_tests.sh

.PHONY: help
help:
	@echo "nh Language Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build the compiler (default)"
	@echo "  make compiler     - Build the nh compiler"
	@echo "  make test-compiler- Test the compiler on examples"
	@echo "  make wasm         - Build game to WASM (requires emscripten)"
	@echo "  make serve        - Start dev server for web frontend"
	@echo "  make serve-build  - Build WASM and start dev server"
	@echo "  make demo         - Open the HTML demo (no WASM needed)"
	@echo "  make clean        - Remove build artifacts"
	@echo ""
	@echo "Requirements:"
	@echo "  - bison (parser generator)"
	@echo "  - flex (lexer generator)"
	@echo "  - gcc (for native compiler build)"
	@echo "  - emscripten (for WASM build)"
