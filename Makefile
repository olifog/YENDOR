# nh - Language + Dungeon Crawler Game
# Build system for the nh compiler and game

# Directories
COMPILER_DIR = compiler
RUNTIME_DIR = runtime
GAME_DIR = game
WEB_DIR = web
BUILD_DIR = build

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
          -s EXPORTED_FUNCTIONS='["_game_init","_game_update","_game_render","_on_key_down","_on_key_up","_on_shift_down","_on_shift_up","_on_char_input","_on_frame_start","_malloc","_free"]' \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s MODULARIZE=1 -s EXPORT_NAME='createModule' \
          -Wno-parentheses-equality -Wno-return-type \
          -lm

# =============================================================================
# Main Targets
# =============================================================================

.PHONY: all
all: compiler

# Quick start: build everything and run dev server
.PHONY: dev
dev: wasm
	@cd $(WEB_DIR) && bun install && bun run dev

# Production build
.PHONY: dist
dist: wasm
	@cd $(WEB_DIR) && bun install && bun run build

# =============================================================================
# Compiler
# =============================================================================

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/parser.tab.h: $(COMPILER_DIR)/parser.y
	cd $(COMPILER_DIR) && $(BISON) -d parser.y

$(COMPILER_DIR)/lex.yy.c: $(COMPILER_DIR)/lexer.l $(COMPILER_DIR)/parser.tab.h
	cd $(COMPILER_DIR) && $(FLEX) lexer.l

.PHONY: compiler
compiler: $(BUILD_DIR) $(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/lex.yy.c
	$(CC) $(CFLAGS) -I$(COMPILER_DIR) \
		$(COMPILER_DIR)/parser.tab.c \
		$(COMPILER_DIR)/lex.yy.c \
		$(COMPILER_DIR)/ast.c \
		$(COMPILER_DIR)/codegen.c \
		$(COMPILER_DIR)/main.c \
		-o $(BUILD_DIR)/dsc

# =============================================================================
# Game (nh → C → WASM)
# =============================================================================

$(BUILD_DIR)/game.c: compiler $(GAME_DIR)/main.nh
	$(BUILD_DIR)/dsc $(GAME_DIR)/main.nh -o $(BUILD_DIR)/game.c

.PHONY: wasm
wasm: $(BUILD_DIR)/game.c
	$(EMCC) $(EMFLAGS) -DGAME_BUILD \
		$(BUILD_DIR)/game.c \
		$(RUNTIME_DIR)/runtime.c \
		-I$(RUNTIME_DIR) \
		-o $(BUILD_DIR)/game.js
	cp $(BUILD_DIR)/game.js $(BUILD_DIR)/game.wasm $(WEB_DIR)/public/

# =============================================================================
# Development
# =============================================================================

.PHONY: serve
serve:
	@cd $(WEB_DIR) && bun run dev

# =============================================================================
# Testing
# =============================================================================

.PHONY: test
test: compiler test-interpreter
	@./tests/run_tests.sh

.PHONY: test-interpreter
test-interpreter: compiler
	$(BUILD_DIR)/dsc interpreter/main.nh > $(BUILD_DIR)/interpreter.c
ifeq ($(shell uname),Darwin)
	$(CC) $(CFLAGS) -I$(RUNTIME_DIR) \
		$(BUILD_DIR)/interpreter.c \
		$(RUNTIME_DIR)/runtime.c \
		-o $(BUILD_DIR)/interpreter \
		-lm -framework OpenGL -framework GLUT
else
	$(CC) $(CFLAGS) -I$(RUNTIME_DIR) \
		$(BUILD_DIR)/interpreter.c \
		$(RUNTIME_DIR)/runtime.c \
		-o $(BUILD_DIR)/interpreter \
		-lm -lGL -lglut -lGLU
endif
	$(BUILD_DIR)/interpreter

# =============================================================================
# Cleanup
# =============================================================================

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(COMPILER_DIR)/parser.tab.c $(COMPILER_DIR)/parser.tab.h
	rm -f $(COMPILER_DIR)/lex.yy.c
	rm -f $(WEB_DIR)/public/game.js $(WEB_DIR)/public/game.wasm
	rm -rf $(WEB_DIR)/dist

# =============================================================================
# Help
# =============================================================================

.PHONY: help
help:
	@echo "nh - Language + Dungeon Crawler"
	@echo ""
	@echo "Quick Start:"
	@echo "  make dev          - Build everything and start dev server"
	@echo ""
	@echo "Build Targets:"
	@echo "  make              - Build the nh compiler"
	@echo "  make wasm         - Compile game to WASM"
	@echo "  make dist         - Production build (WASM + bundle)"
	@echo ""
	@echo "Development:"
	@echo "  make serve        - Start Vite dev server (assumes WASM built)"
	@echo "  make test         - Run test suite (includes interpreter)"
	@echo "  make clean        - Remove build artifacts"
	@echo ""
	@echo "Requirements:"
	@echo "  bison, flex, gcc, emscripten, bun"
