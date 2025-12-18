#define GAME_BUILD // Mock game build to avoid missing symbols if any
#include "../runtime/runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock runtime.c inclusion to get access to static functions/internal state if
// needed Or just link against it. For simplicity, let's include it directly?
// No, that's messy. Let's just use the public API and maybe some "friend" logic
// if needed. actually runtime.c is not a library, it's usually compiled with
// the game. If I want to test internal behavior like "gc_collect", I might need
// to declare it.

// extern void gc_collect(void); // It's static...
// extern void gc_maybe_collect(void); // static...

// BUT, runtime.c exposes `ds_object_create` etc.
// And checking `ensure_gl_context` issues... I'll need to mock GLUT/GL if not
// careful. runtime.c includes runtime.h.

// Let's rely on the fact that `ds_object_create` fails (returns 0) if it can't
// allocate. And `gc_maybe_collect` is called... wait, where is it called?
// `on_frame_start` calls `gc_maybe_collect`.

// So the loop should be:
// 1. Create Garbage
// 2. Call `on_frame_start()` to trigger GC.
// 3. Repeat.

/* Implement missing symbols required by runtime.c if it assumes it's linked
 * with other things? */
/* runtime.c provides most things. */

int main() {
  printf("Starting GC Saturation Stress Test...\n");

// Create a root array to hold 20,000 strings
// This exceeds GC_HASH_SIZE (16384)
#define TEST_SIZE 20000
  Value *roots = (Value *)malloc(TEST_SIZE * sizeof(Value));
  if (!roots) {
    printf("Failed to allocate roots array\n");
    return 1;
  }
  memset(roots, 0, TEST_SIZE * sizeof(Value));

  // Register as GC root
  gc_register_root_array(roots, TEST_SIZE);

  printf("Allocating %d strings...\n", TEST_SIZE);
  for (int i = 0; i < TEST_SIZE; i++) {
    // Create a unique string for each slot via object property key
    Value obj = ds_object_create(VAL_INT(0));
    if (AS_INT(obj) == 0) {
      printf("Failed to create object %d\n", i);
      return 1;
    }

    // Set a property. This allocates a string for the key "prop".
    // "prop" keys are duplicated allocations in current runtime
    ds_set_prop(obj, VAL_OBJ("a"), VAL_INT(i));

    // Store object handle in roots to keep it alive
    roots[i] = obj;
  }

  printf("Allocations done. Triggering GC...\n");

  // Trigger GC (Force it)
  on_frame_start();

  printf("GC done. Verifying objects...\n");

  // Verify all objects and their keys are still valid
  for (int i = 0; i < TEST_SIZE; i++) {
    Value obj = roots[i];
    // This get will compare the key "a" (literal) with the stored key
    // If stored key was freed, we crash or get garbage
    Value val = ds_object_get(obj, VAL_OBJ("a"));
    if (AS_INT(val) != i) {
      printf("FAILED: Object %d lost its property! Got %ld, expected %d\n", i,
             AS_INT(val), i);
      return 1;
    }
  }

  printf("SUCCESS: All objects survived GC.\n");
  free(roots);
  return 0;
}
