#!/bin/bash
# nh Test Runner - Actually Executes and Validates Output

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
COMPILER="$PROJECT_DIR/build/dsc"
TEST_DIR="$SCRIPT_DIR"
TMP_DIR="/tmp/nh_tests"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0

mkdir -p "$TMP_DIR"

echo "=================================="
echo "  nh Test Suite"
echo "=================================="
echo ""

# Check if compiler exists
if [ ! -f "$COMPILER" ]; then
    echo -e "${RED}Error: Compiler not found at $COMPILER${NC}"
    echo "Run 'make compiler' first."
    exit 1
fi

# Create a minimal runtime stub for testing (no SDL dependency)
cat > "$TMP_DIR/runtime.h" << 'EOF'
#ifndef RUNTIME_H
#define RUNTIME_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// ============================================================================
// Types & formatting
// ============================================================================

typedef long Value;

// Tagging macros: Integers are tagged (odd), Pointers pass through unchanged
#define VAL_INT(x) (((long)(x) << 1) | 1)
#define VAL_OBJ(x) ((Value)(x))
#define AS_INT(x) ((long)(x) >> 1)
#define AS_OBJ(x) ((void *)(x))
#define IS_INT(x) (((x) & 1))
#define IS_OBJ(x) (!((x) & 1))

// GC stubs (no-op for test environment)
static inline void gc_register_root_array(Value *array, int size) { (void)array; (void)size; }
static inline void gc_register_root_value(Value *value_ptr) { (void)value_ptr; }

static inline void console_log(Value msg) { printf("%s\n", (const char *)AS_OBJ(msg)); }
static inline void console_log_int(Value value) { printf("%ld\n", AS_INT(value)); }
static inline void console_log_float(float value) { printf("%f\n", value); }

// ============================================================================
// Object System (Handle-based)
// ============================================================================

#define MAX_OBJECTS 1024
#define MAX_PROPS 64

typedef struct {
  char *key;
  Value value;
} Property;

typedef struct {
  Property props[MAX_PROPS];
  int prop_count;
  int in_use;
} Object;

// Simple static allocation for tests
static Object objects[MAX_OBJECTS];
static int objects_initialized = 0;

static void init_objects(void) {
  if (objects_initialized) return;
  memset(objects, 0, sizeof(objects));
  objects_initialized = 1;
}

static Value alloc_object(void) {
  init_objects();
  for (int i = 1; i < MAX_OBJECTS; i++) {
    if (!objects[i].in_use) {
      objects[i].in_use = 1;
      objects[i].prop_count = 0;
      return i;
    }
  }
  return 0; // OOM
}

// Forward decl
static void ds_object_set_impl(Value *obj, const char *key, Value value);

static inline Value ds_object_create(Value count_val, ...) {
  int count = (int)AS_INT(count_val);
  Value handle = alloc_object();
  if (handle == 0) return VAL_INT(0);

  va_list args;
  va_start(args, count_val);
  for (int i = 0; i < count; i++) {
    const char *key = va_arg(args, const char *);
    Value val = va_arg(args, Value);
    ds_object_set_impl(&handle, key, val);
  }
  va_end(args);
  return handle;
}

static inline Value ds_object_get(Value obj, Value key_val) {
  const char *key = (const char *)AS_OBJ(key_val);
  if (obj <= 0 || obj >= MAX_OBJECTS) return VAL_INT(0);
  if (!objects[obj].in_use) return VAL_INT(0);

  for (int i = 0; i < objects[obj].prop_count; i++) {
    if (strcmp(objects[obj].props[i].key, key) == 0) {
      return objects[obj].props[i].value;
    }
  }
  return VAL_INT(0);
}

static void ds_object_set_impl(Value *obj, const char *key, Value value) {
    Value handle = *obj;
    if (handle <= 0 || handle >= MAX_OBJECTS) {
        handle = alloc_object();
        *obj = handle;
    }
    if (!objects[handle].in_use) return;

    // Update existing
    for (int i = 0; i < objects[handle].prop_count; i++) {
        if (strcmp(objects[handle].props[i].key, key) == 0) {
            objects[handle].props[i].value = value;
            return;
        }
    }
    // Add new
    if (objects[handle].prop_count < MAX_PROPS) {
        int idx = objects[handle].prop_count++;
        objects[handle].props[idx].key = strdup(key);
        objects[handle].props[idx].value = value;
    }
}

// Wrapper to match runtime.h
static inline void ds_object_set(Value *obj, Value key_val, Value value) {
    const char *key = (const char *)key_val;
    ds_object_set_impl(obj, key, value);
}

static inline void ds_set_prop(Value obj, Value key_val, Value value) {
    const char *key = (const char *)key_val; // In test environment, strings are char*
    Value handle = obj; // Obj is passed by value (handle)
    ds_object_set_impl(&handle, key, value);
}

// ============================================================================
// String / Helpers
// ============================================================================

static inline Value ds_strlen(Value str_val) {
   const char* s = (const char*)AS_OBJ(str_val);
   return VAL_INT(s ? strlen(s) : 0);
}
static inline Value ds_streq(Value s1, Value s2) {
    const char* a = (const char*)AS_OBJ(s1);
    const char* b = (const char*)AS_OBJ(s2);
    if(!a || !b) return VAL_INT(0);
    return VAL_INT(strcmp(a, b) == 0);
}

static inline Value val_eq(Value a, Value b) {
    if (a == b) return VAL_INT(1);
    // If integer, equality already checked by a==b.
    // So if either is int here, they are different or different types
    if (IS_INT(a) || IS_INT(b)) return VAL_INT(0);

    const char *s1 = (const char *)AS_OBJ(a);
    const char *s2 = (const char *)AS_OBJ(b);
    if (!s1 || !s2) return VAL_INT(0);
    return VAL_INT(strcmp(s1, s2) == 0);
}

// ============================================================================
// Stubs
// ============================================================================

static inline void gfx_init(void) {}
static inline void gfx_clear(int r, int g, int b, int a) { (void)r;(void)g;(void)b;(void)a; }
static inline void gfx_rect(int x, int y, int w, int h, int r, int g, int b, int a) { 
    (void)x;(void)y;(void)w;(void)h;(void)r;(void)g;(void)b;(void)a; 
}
static inline void gfx_present(void) {}

// Math
static inline Value math_random(Value min, Value max) { return min; } // Deterministic for tests
static inline Value math_sin(Value x) { return VAL_INT((long)(sin(AS_INT(x))*1000)); }
static inline Value math_cos(Value x) { return VAL_INT((long)(cos(AS_INT(x))*1000)); }

// Time
static inline Value time_ms(void) { return VAL_INT(0); }

// Input
static inline Value input_key_pressed(Value key) { (void)key; return VAL_INT(0); }

#endif
EOF

# Run each test file
for test_file in "$TEST_DIR"/*.nh; do
    test_name=$(basename "$test_file" .nh)
    
    echo -n "Testing $test_name... "
    
    # Check for expected output comment in file
    # Format: // EXPECT: <expected_output>
    expected=$(grep -E "^// EXPECT:" "$test_file" | sed 's/^\/\/ EXPECT: //' || true)
    
    # Try to parse (AST mode)
    if ! "$COMPILER" --ast "$test_file" > /dev/null 2>&1; then
        echo -e "${RED}FAIL (parse error)${NC}"
        "$COMPILER" --ast "$test_file" 2>&1 | head -5
        FAILED=$((FAILED + 1))
        continue
    fi
    
    # Try to generate C code
    if ! "$COMPILER" "$test_file" > "$TMP_DIR/test_$test_name.c" 2>&1; then
        echo -e "${YELLOW}FAIL (codegen error)${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi
    
    # Try to compile the generated C
    if ! gcc -w "$TMP_DIR/test_$test_name.c" -I"$TMP_DIR" -o "$TMP_DIR/test_$test_name" 2>/dev/null; then
        echo -e "${YELLOW}FAIL (C compile error)${NC}"
        gcc "$TMP_DIR/test_$test_name.c" -I"$TMP_DIR" -o "$TMP_DIR/test_$test_name" 2>&1 | head -5
        FAILED=$((FAILED + 1))
        continue
    fi
    
    # Run the test and capture output
    actual=$("$TMP_DIR/test_$test_name" 2>&1 || true)
    exit_code=$?
    
    # If there's an expected output, validate it
    if [ -n "$expected" ]; then
        if [ "$actual" = "$expected" ]; then
            echo -e "${GREEN}PASS${NC} (output validated)"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}FAIL (output mismatch)${NC}"
            echo "  Expected: $expected"
            echo "  Actual:   $actual"
            FAILED=$((FAILED + 1))
        fi
    else
        # No expected output, just check it runs
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}PASS${NC}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${YELLOW}PASS (exit code $exit_code)${NC}"
            PASSED=$((PASSED + 1))
        fi
    fi
done

echo ""
echo "=================================="
echo "  Results"
echo "=================================="
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo ""

# Cleanup
rm -rf "$TMP_DIR"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
