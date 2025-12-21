#define GL_GLEXT_PROTOTYPES
#define GL_SILENCE_DEPRECATION
#include "runtime.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl3.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Garbage Collector
// ============================================================================

// Type Masks for Handles (Distinguish Integers from Handles)
// 0x10000000 = Object Handle (Bit 28)
// 0x20000000 = List Handle (Bit 29)
// These bits are safe in signed 32-bit positive integers even after << 1
// tagging
#define TYPE_MASK_OBJ 0x10000000
#define TYPE_MASK_LIST 0x20000000

#define GC_MAX_ALLOCATIONS 131072 // Max tracked allocations

typedef enum {
  GC_TYPE_STRING,      // Heap-allocated string
  GC_TYPE_LIST_ITEMS,  // List items array
  GC_TYPE_FLOAT_BUFFER // Float buffer data
} GcAllocType;

typedef struct {
  void *ptr;        // Pointer to allocated memory
  GcAllocType type; // Type of allocation
  int marked;       // Mark bit for current GC cycle
  int in_use;       // Is this slot in use?
} GcAllocation;

static GcAllocation gc_allocations[GC_MAX_ALLOCATIONS];
static int gc_initialized = 0;
static int gc_allocation_count = 0;
static int gc_threshold = 1000;    // Trigger GC after this many allocations
static int gc_next_alloc_slot = 0; // Cursor for next-fit allocation

// IMPORTANT: GC requires roots to be registered. Compiled nh code stores
// strings in C global variables and arrays. These MUST be registered via
// gc_register_root_array() and gc_register_root_value() for safe collection.

// Root array registry - for arrays like editor_lines[]
#define GC_MAX_ROOT_ARRAYS 1024

typedef struct {
  Value *array; // Pointer to array start
  int size;     // Number of elements
  int in_use;
} GcRootArray;

static GcRootArray gc_root_arrays[GC_MAX_ROOT_ARRAYS];

// Root value registry - for single globals like vim_yank_buffer
#define GC_MAX_ROOT_VALUES 1024

static Value *gc_root_values[GC_MAX_ROOT_VALUES];
static int gc_root_value_count = 0;

// Register an array as a GC root (called from generated code)
void gc_register_root_array(Value *array, int size) {
  for (int i = 0; i < GC_MAX_ROOT_ARRAYS; i++) {
    if (!gc_root_arrays[i].in_use) {
      gc_root_arrays[i].array = array;
      gc_root_arrays[i].size = size;
      gc_root_arrays[i].in_use = 1;
      return;
    }
  }
}

// Register a single value as a GC root (called from generated code)
void gc_register_root_value(Value *value_ptr) {
  if (gc_root_value_count < GC_MAX_ROOT_VALUES) {
    gc_root_values[gc_root_value_count++] = value_ptr;
  } else {
    printf("[GC ERROR] gc_register_root_value: capacity exceeded! Max=%d\n",
           GC_MAX_ROOT_VALUES);
  }
}

// ============================================================================
// Execution Stack Protection
// Protects temporary environments (e.g., function call envs) during execution
// ============================================================================

#define GC_MAX_EXEC_STACK 512

static Value gc_exec_stack[GC_MAX_EXEC_STACK];
static int gc_exec_stack_depth = 0;

// Push an environment onto the execution stack to protect it from GC
void gc_push_env(Value env) {
  if (gc_exec_stack_depth < GC_MAX_EXEC_STACK) {
    gc_exec_stack[gc_exec_stack_depth++] = env;
  }
}

// Pop an environment from the execution stack
void gc_pop_env(void) {
  if (gc_exec_stack_depth > 0) {
    gc_exec_stack_depth--;
  }
}

// Clear the execution stack (call when interpreter stops)
void gc_clear_exec_stack(void) { gc_exec_stack_depth = 0; }

// Clear a root array (set all elements to 0 so GC can collect old objects)
void gc_clear_array(Value *array, Value size_val) {
  int size = AS_INT(size_val);
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

static void gc_init(void) {
  if (gc_initialized)
    return;
  memset(gc_allocations, 0, sizeof(gc_allocations));
  gc_initialized = 1;
}

// Forward declarations for hash table (defined after gc_alloc)
static void gc_hash_insert(void *ptr, int slot);
static void gc_hash_remove(void *ptr);

// Register an allocation with the GC
static void *gc_alloc(size_t size, GcAllocType type) {
  gc_init();

  void *ptr = malloc(size);
  if (!ptr)
    return NULL;

  // Find a free slot (Next Fit strategy for O(1) amortized)
  int start_slot = gc_next_alloc_slot;
  int i = start_slot;

  do {
    if (!gc_allocations[i].in_use) {
      gc_allocations[i].ptr = ptr;
      gc_allocations[i].type = type;
      gc_allocations[i].marked = 0;
      gc_allocations[i].in_use = 1;
      gc_allocation_count++;
      gc_hash_insert(ptr, i); // Add to hash table

      // Update cursor for next time
      gc_next_alloc_slot = (i + 1) % GC_MAX_ALLOCATIONS;
      return ptr;
    }
    i = (i + 1) % GC_MAX_ALLOCATIONS;
  } while (i != start_slot);

  // No slots available, just leak (safety over efficiency)
  return ptr;
}

// ============================================================================
// Hash table for fast pointer -> slot lookup
// ============================================================================

#define GC_HASH_SIZE 262144 // Must be power of 2, and > GC_MAX_ALLOCATIONS

typedef struct {
  void *ptr;
  int slot;
} GcHashEntry;

static GcHashEntry gc_hash_table[GC_HASH_SIZE];
static int gc_hash_initialized = 0;

static void gc_hash_init(void) {
  if (gc_hash_initialized)
    return;
  memset(gc_hash_table, 0, sizeof(gc_hash_table));
  gc_hash_initialized = 1;
}

static unsigned int gc_hash_ptr(void *ptr) {
  // Better mixing function for pointer hashing
  uintptr_t x = (uintptr_t)ptr;
  x = (x ^ (x >> 16)) * 0x45d9f3b;
  x = (x ^ (x >> 16)) * 0x45d9f3b;
  x = x ^ (x >> 16);
  return (unsigned int)(x & (GC_HASH_SIZE - 1));
}

static void gc_hash_insert(void *ptr, int slot) {
  gc_hash_init();
  unsigned int h = gc_hash_ptr(ptr);
  // Linear probing
  for (int i = 0; i < GC_HASH_SIZE; i++) {
    unsigned int idx = (h + i) & (GC_HASH_SIZE - 1);
    if (gc_hash_table[idx].ptr == NULL || gc_hash_table[idx].ptr == ptr) {
      gc_hash_table[idx].ptr = ptr;
      gc_hash_table[idx].slot = slot;
      return;
    }
  }
  // Hash table full - shouldn't happen if GC_HASH_SIZE >= GC_MAX_ALLOCATIONS
}

static void gc_hash_remove(void *ptr) {
  unsigned int h = gc_hash_ptr(ptr);
  int found_idx = -1;
  for (int i = 0; i < GC_HASH_SIZE; i++) {
    unsigned int idx = (h + i) & (GC_HASH_SIZE - 1);
    if (gc_hash_table[idx].ptr == ptr) {
      found_idx = idx;
      break;
    }
    if (gc_hash_table[idx].ptr == NULL) {
      return; // Not found
    }
  }

  if (found_idx == -1)
    return;

  // Clear the found slot
  gc_hash_table[found_idx].ptr = NULL;
  gc_hash_table[found_idx].slot = -1;

  // Re-hash the following cluster to fill the hole
  int j = found_idx;
  while (1) {
    j = (j + 1) & (GC_HASH_SIZE - 1);
    if (gc_hash_table[j].ptr == NULL)
      break;

    void *rehash_ptr = gc_hash_table[j].ptr;
    int rehash_slot = gc_hash_table[j].slot;
    gc_hash_table[j].ptr = NULL;
    gc_hash_table[j].slot = -1;
    gc_hash_insert(rehash_ptr, rehash_slot);
  }
}

// Find GC slot for a pointer using hash table (O(1) average)
static int gc_find_slot(void *ptr) {
  if (!ptr)
    return -1;
  gc_hash_init();
  unsigned int h = gc_hash_ptr(ptr);
  for (int i = 0; i < GC_HASH_SIZE; i++) {
    unsigned int idx = (h + i) & (GC_HASH_SIZE - 1);
    if (gc_hash_table[idx].ptr == ptr) {
      // Verify the slot is still valid
      int slot = gc_hash_table[idx].slot;
      if (slot >= 0 && slot < GC_MAX_ALLOCATIONS &&
          gc_allocations[slot].in_use && gc_allocations[slot].ptr == ptr) {
        return slot;
      }
      return -1; // Stale entry
    }
    if (gc_hash_table[idx].ptr == NULL) {
      return -1; // Not found
    }
  }
  return -1;
}

// Mark a pointer as reachable
static void gc_mark_ptr(void *ptr) {
  int slot = gc_find_slot(ptr);
  if (slot >= 0) {
    gc_allocations[slot].marked = 1;
  }
}

// Forward declarations for mark phase (defined later)
static void gc_mark(void);
static void gc_sweep(void);

// Run a full GC cycle
static void gc_collect(void) {
  gc_mark();
  gc_sweep();
}

// Check if GC should run
static void gc_maybe_collect(void) {
  if (gc_allocation_count >= gc_threshold) {
    gc_collect();
  }
}

// Public function to force a GC cycle (called when bot stops)
void gc_force_collect(void) { gc_collect(); }

// Unregister an allocation (for manual free)
static void gc_unregister(void *ptr) {
  int slot = gc_find_slot(ptr);
  if (slot >= 0) {
    gc_hash_remove(ptr); // Remove from hash table
    gc_allocations[slot].in_use = 0;
    gc_allocations[slot].ptr = NULL;
    gc_allocation_count--;
  }
}

// ============================================================================
// IO
// ============================================================================

void text_write(const char *str) { printf("%s", str); }

void text_write_int(Value val) { printf("%ld", AS_INT(val)); }

Value time_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return VAL_INT((long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000));
}

// ============================================================================
// Object System Implementation
// ============================================================================

#define MAX_OBJECTS 65536
#define MAX_PROPS 256

typedef struct {
  char *key;
  Value value;
} Property;

typedef struct {
  Property props[MAX_PROPS];
  int prop_count;
  int in_use;
  int marked; // For GC
} Object;

static Object objects[MAX_OBJECTS];
static int objects_initialized = 0;

static void init_objects(void) {
  if (objects_initialized)
    return;
  memset(objects, 0, sizeof(objects));
  objects_initialized = 1;
}

static int alloc_object_idx(void) {
  init_objects();
  for (int i = 1; i < MAX_OBJECTS; i++) {
    if (!objects[i].in_use) {
      objects[i].in_use = 1;
      objects[i].marked = 0;
      objects[i].prop_count = 0;
      return i;
    }
  }
  return 0;
}

Value ds_object_create(Value count_val, ...) {
  int count = (int)AS_INT(count_val);
  int idx = alloc_object_idx();
  if (idx == 0)
    return VAL_INT(0);

  Value handle = VAL_INT(idx | TYPE_MASK_OBJ);

  va_list args;
  va_start(args, count_val);

  for (int i = 0; i < count; i++) {
    const char *key = va_arg(args, const char *);
    Value val = va_arg(args, Value);
    ds_object_set(&handle, VAL_OBJ(key), val);
  }

  va_end(args);
  return handle;
}

Value ds_object_get(Value obj_val, Value key_val) {
  const char *key = (const char *)AS_OBJ(key_val);
  long handle = AS_INT(obj_val);

  // Validate Object Mask
  if ((handle & TYPE_MASK_OBJ) != TYPE_MASK_OBJ)
    return VAL_INT(0);
  long obj = handle & ~TYPE_MASK_OBJ;

  if (obj <= 0 || obj >= MAX_OBJECTS)
    return VAL_INT(0);
  if (!objects[obj].in_use)
    return VAL_INT(0);

  for (int i = 0; i < objects[obj].prop_count; i++) {
    if (strcmp(objects[obj].props[i].key, key) == 0) {
      return objects[obj].props[i].value;
    }
  }
  return VAL_INT(0);
}

void ds_object_set(Value *obj, Value key_val, Value value) {
  const char *key = (const char *)AS_OBJ(key_val);
  Value handle_val = *obj;
  long handle = AS_INT(handle_val);

  // Auto-allocate if null/zero
  if (handle == 0) {
    int idx = alloc_object_idx();
    if (idx == 0)
      return;
    handle = idx | TYPE_MASK_OBJ;
    *obj = VAL_INT(handle);
  } else {
    // Validate existing handle
    if ((handle & TYPE_MASK_OBJ) != TYPE_MASK_OBJ)
      return; // Not an object, ignore
    handle = handle & ~TYPE_MASK_OBJ;
  }

  if (handle <= 0 || handle >= MAX_OBJECTS)
    return;

  if (!objects[handle].in_use) {
    return;
  }

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
    // Use gc_alloc instead of strdup for GC tracking
    size_t key_len = strlen(key) + 1;
    char *key_copy = gc_alloc(key_len, GC_TYPE_STRING);
    if (key_copy) {
      memcpy(key_copy, key, key_len);
      objects[handle].props[idx].key = key_copy;
    } else {
      objects[handle].props[idx].key = NULL;
    }
    objects[handle].props[idx].value = value;
  }
}

void ds_set_prop(Value obj_val, Value key_val, Value value) {
  long handle = AS_INT(obj_val);
  // Validate mask before passing address (though ds_object_set checks too)
  if ((handle & TYPE_MASK_OBJ) != TYPE_MASK_OBJ)
    return;

  // We need to pass a pointer, but obj_val itself is the handle
  ds_object_set(&obj_val, key_val, value);
}

Value ds_strlen(Value str_val) {
  // Use AS_OBJ to get pointer value (clears lowest bit if any)
  const char *str = (const char *)AS_OBJ(str_val);
  if (!str)
    return VAL_INT(0);
  return VAL_INT((long)strlen(str));
}
Value ds_streq(Value s1_val, Value s2_val) {
  const char *s1 = (const char *)AS_OBJ(s1_val);
  const char *s2 = (const char *)AS_OBJ(s2_val);
  if (!s1 || !s2)
    return VAL_INT(0);
  return VAL_INT(strcmp(s1, s2) == 0);
}

Value val_eq(Value a, Value b) {
  if (a == b)
    return VAL_INT(1);
  if (IS_INT(a) || IS_INT(b))
    return VAL_INT(0);
  const char *s1 = (const char *)AS_OBJ(a);
  const char *s2 = (const char *)AS_OBJ(b);
  if (!s1 || !s2)
    return VAL_INT(0);
  return VAL_INT(strcmp(s1, s2) == 0);
}

Value ds_div(Value a, Value b) {
  long ia = AS_INT(a);
  long ib = AS_INT(b);
  if (ib == 0)
    return VAL_INT(0);
  return VAL_INT(ia / ib);
}

Value ds_mod(Value a, Value b) {
  long ia = AS_INT(a);
  long ib = AS_INT(b);
  if (ib == 0)
    return VAL_INT(0);
  return VAL_INT(ia % ib);
}

Value ds_is_string_like(Value val) {
  // With tagging: integers have bit 0 = 1, pointers have bit 0 = 0
  // A string is a pointer (even value), so IS_OBJ(val) tells us it's a pointer.
  // However, we should also check if it's actually a string vs other obj types.
  // For now, any non-integer is considered "string-like" for this heuristic.
  return VAL_INT(!IS_INT(val));
}

// Robust check: is this value a string?
Value ds_is_string(Value val) {
  // With tagging: integers are odd (bit 0 = 1), pointers are even (bit 0 = 0)
  return VAL_INT(IS_OBJ(val));
}
Value ds_substring(Value str_val, Value start_val, Value len_val) {
  const char *str = (const char *)AS_OBJ(str_val);
  long start = AS_INT(start_val);
  long len = AS_INT(len_val);
  if (!str || len <= 0)
    return VAL_OBJ("");
  size_t str_len = strlen(str);
  if (start >= (long)str_len)
    return VAL_OBJ("");
  if (start + len > (long)str_len)
    len = str_len - start;
  char *sub = gc_alloc(len + 1, GC_TYPE_STRING);
  if (!sub)
    return VAL_OBJ("");
  strncpy(sub, str + start, len);
  sub[len] = '\0';
  return VAL_OBJ(sub);
}

Value ds_string_at(Value str_val, Value index_val) {
  const char *str = (const char *)AS_OBJ(str_val);
  long index = AS_INT(index_val);
  if (!str)
    return VAL_INT(0);
  if (index < 0 || index >= (long)strlen(str))
    return VAL_INT(0);
  return VAL_INT((long)str[index]);
}

// Insert a character at position in string, returns new string
Value ds_string_insert_char(Value str_val, Value pos_val, Value char_code_val) {
  const char *str = (const char *)AS_OBJ(str_val);
  long pos = AS_INT(pos_val);
  long char_code = AS_INT(char_code_val);
  if (!str)
    str = "";
  size_t len = strlen(str);
  if (pos < 0)
    pos = 0;
  if (pos > (long)len)
    pos = len;

  char *result = gc_alloc(len + 2, GC_TYPE_STRING);
  if (!result)
    return str_val;
  strncpy(result, str, pos);
  result[pos] = (char)char_code;
  strcpy(result + pos + 1, str + pos);
  return VAL_OBJ(result);
}

// Delete character at position in string, returns new string
Value ds_string_delete_char(Value str_val, Value pos_val) {
  const char *str = (const char *)AS_OBJ(str_val);
  long pos = AS_INT(pos_val);
  if (!str)
    return VAL_OBJ("");
  size_t len = strlen(str);
  if (pos < 0 || pos >= (long)len)
    return str_val;

  char *result = gc_alloc(len, GC_TYPE_STRING);
  if (!result)
    return str_val;
  strncpy(result, str, pos);
  strcpy(result + pos, str + pos + 1);
  return VAL_OBJ(result);
}

// Get string length
Value ds_string_length(Value str_val) {
  const char *str = (const char *)AS_OBJ(str_val);
  if (!str)
    return VAL_INT(0);
  return VAL_INT(strlen(str));
}

// Convert integer to string
Value ds_int_to_string(Value int_val) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%ld", AS_INT(int_val));

  size_t len = strlen(buf);
  char *result = gc_alloc(len + 1, GC_TYPE_STRING);
  if (!result)
    return VAL_OBJ("");
  strcpy(result, buf);
  return VAL_OBJ(result);
}

// Concatenate two strings
Value ds_string_concat(Value str1_val, Value str2_val) {
  const char *str1 = (const char *)AS_OBJ(str1_val);
  const char *str2 = (const char *)AS_OBJ(str2_val);
  if (!str1)
    str1 = "";
  if (!str2)
    str2 = "";

  size_t len1 = strlen(str1);
  size_t len2 = strlen(str2);
  char *result = gc_alloc(len1 + len2 + 1, GC_TYPE_STRING);
  if (!result)
    return str1_val;
  strcpy(result, str1);
  strcpy(result + len1, str2);
  return VAL_OBJ(result);
}

// Create a single-character string from char code
Value ds_char_to_string(Value char_code_val) {
  long char_code = AS_INT(char_code_val);
  char *result = gc_alloc(2, GC_TYPE_STRING);
  if (!result)
    return VAL_OBJ("");
  result[0] = (char)char_code;
  result[1] = '\0';
  return VAL_OBJ(result);
}

// ============================================================================
// List System Implementation
// ============================================================================

#define MAX_LISTS 65536

typedef struct {
  Value *items;
  int count;
  int capacity;
  int in_use;
  int marked; // For GC
} List;

static List lists[MAX_LISTS];
static int lists_initialized = 0;

static void init_lists(void) {
  if (lists_initialized)
    return;
  memset(lists, 0, sizeof(lists));
  lists_initialized = 1;
}

Value ds_list_create(void) {
  init_lists();
  for (int i = 1; i < MAX_LISTS; i++) {
    if (!lists[i].in_use) {
      lists[i].in_use = 1;
      lists[i].marked = 0;
      lists[i].count = 0;
      lists[i].capacity = 16;
      lists[i].items = (Value *)gc_alloc(lists[i].capacity * sizeof(Value),
                                         GC_TYPE_LIST_ITEMS);
      return VAL_INT(i | TYPE_MASK_LIST);
    }
  }
  return VAL_INT(0);
}

Value ds_list_push(Value list_val, Value value) {
  long handle = AS_INT(list_val);
  if ((handle & TYPE_MASK_LIST) != TYPE_MASK_LIST)
    return VAL_INT(0);
  long list = handle & ~TYPE_MASK_LIST;

  if (list <= 0 || list >= MAX_LISTS)
    return VAL_INT(0);
  if (!lists[list].in_use)
    return VAL_INT(0);

  if (lists[list].count >= lists[list].capacity) {
    // Allocate new larger buffer via GC
    int new_capacity = lists[list].capacity * 2;
    Value *new_items =
        (Value *)gc_alloc(new_capacity * sizeof(Value), GC_TYPE_LIST_ITEMS);
    if (new_items) {
      // Copy old items
      memcpy(new_items, lists[list].items, lists[list].count * sizeof(Value));
      // Unregister old allocation from GC and free it
      gc_unregister(lists[list].items);
      free(lists[list].items);
      lists[list].items = new_items;
      lists[list].capacity = new_capacity;
    } else {
      // Allocation failed, cannot add item
      return VAL_INT(0);
    }
  }
  lists[list].items[lists[list].count++] = value;
  return VAL_INT(0);
}

Value ds_list_get(Value list_val, Value index_val) {
  long handle = AS_INT(list_val);
  if ((handle & TYPE_MASK_LIST) != TYPE_MASK_LIST)
    return VAL_INT(0);
  long list = handle & ~TYPE_MASK_LIST;

  long index = AS_INT(index_val);
  if (list <= 0 || list >= MAX_LISTS)
    return VAL_INT(0);
  if (!lists[list].in_use)
    return VAL_INT(0);
  if (index < 0 || index >= lists[list].count)
    return VAL_INT(0);

  return lists[list].items[index];
}

Value ds_list_len(Value list_val) {
  long handle = AS_INT(list_val);
  if ((handle & TYPE_MASK_LIST) != TYPE_MASK_LIST)
    return VAL_INT(0);
  long list = handle & ~TYPE_MASK_LIST;

  if (list <= 0 || list >= MAX_LISTS)
    return VAL_INT(0);
  if (!lists[list].in_use)
    return VAL_INT(0);
  return VAL_INT(lists[list].count);
}

// Check if a value is a valid list handle
Value ds_is_list(Value val) {
  if (!IS_INT(val))
    return VAL_INT(0);
  long handle = AS_INT(val);
  if ((handle & TYPE_MASK_LIST) != TYPE_MASK_LIST)
    return VAL_INT(0);
  long id = handle & ~TYPE_MASK_LIST;

  if (id <= 0 || id >= MAX_LISTS)
    return VAL_INT(0);
  return VAL_INT(lists[id].in_use);
}

// Check if a value is a valid object handle
Value ds_is_object(Value val) {
  if (!IS_INT(val))
    return VAL_INT(0);
  long handle = AS_INT(val);
  if ((handle & TYPE_MASK_OBJ) != TYPE_MASK_OBJ)
    return VAL_INT(0);
  long id = handle & ~TYPE_MASK_OBJ;

  if (id <= 0 || id >= MAX_OBJECTS)
    return VAL_INT(0);
  return VAL_INT(objects[id].in_use);
}

// Convert a list to a string representation "[a, b, c]"
// Handles nested lists recursively with depth limit
// Helper for recursion
static void val_to_string_recursive(Value val, char *buf, int buf_size,
                                    int *pos, int depth, int force_type) {
  if (depth > 10) {
    int len = snprintf(buf + *pos, buf_size - *pos, "...");
    *pos += len;
    return;
  }

  if (!IS_INT(val)) {
    // String
    const char *str = (const char *)AS_OBJ(val);
    if (str) {
      int len = snprintf(buf + *pos, buf_size - *pos, "\"%s\"", str);
      *pos += len;
    } else {
      int len = snprintf(buf + *pos, buf_size - *pos, "null");
      *pos += len;
    }
    return;
  }

  long id = AS_INT(val);

  long obj_idx = -1;
  long list_idx = -1;

  if ((id & TYPE_MASK_OBJ) == TYPE_MASK_OBJ) {
    obj_idx = id & ~TYPE_MASK_OBJ;
  } else if ((id & TYPE_MASK_LIST) == TYPE_MASK_LIST) {
    list_idx = id & ~TYPE_MASK_LIST;
  }

  // Auto Mode includes 0 and legacy 3
  int is_auto = (force_type == 0 || force_type == 3);

  // Try Object
  if ((force_type == 1) || (is_auto && obj_idx != -1)) {
    long idx = (force_type == 1) ? id : obj_idx;
    // Unwrap if forced handle provided
    if ((idx & TYPE_MASK_OBJ) == TYPE_MASK_OBJ)
      idx &= ~TYPE_MASK_OBJ;

    if (idx > 0 && idx < MAX_OBJECTS && objects[idx].in_use) {
      int len = snprintf(buf + *pos, buf_size - *pos, "{");
      *pos += len;

      for (int i = 0; i < objects[idx].prop_count && *pos < buf_size - 10;
           i++) {
        if (i > 0) {
          len = snprintf(buf + *pos, buf_size - *pos, ", ");
          *pos += len;
        }
        len = snprintf(buf + *pos, buf_size - *pos,
                       "%s: ", objects[idx].props[i].key);
        *pos += len;
        // Recurse with Auto
        val_to_string_recursive(objects[idx].props[i].value, buf, buf_size, pos,
                                depth + 1, 0);
      }
      len = snprintf(buf + *pos, buf_size - *pos, "}");
      *pos += len;
      return;
    }
  }

  // Try List
  if ((force_type == 2) || (is_auto && list_idx != -1)) {
    long idx = (force_type == 2) ? id : list_idx;
    // Unwrap
    if ((idx & TYPE_MASK_LIST) == TYPE_MASK_LIST)
      idx &= ~TYPE_MASK_LIST;

    if (idx > 0 && idx < MAX_LISTS && lists[idx].in_use) {
      snprintf(buf + *pos, buf_size - *pos, "[");
      *pos += strlen(buf + *pos);

      for (int i = 0; i < lists[idx].count && *pos < buf_size - 10; i++) {
        if (i > 0) {
          snprintf(buf + *pos, buf_size - *pos, ", ");
          *pos += strlen(buf + *pos);
        }
        val_to_string_recursive(lists[idx].items[i], buf, buf_size, pos,
                                depth + 1, 0);
      }
      snprintf(buf + *pos, buf_size - *pos, "]");
      *pos += strlen(buf + *pos);
      return;
    }
  }

  // Plain Integer
  int len = snprintf(buf + *pos, buf_size - *pos, "%ld", id);
  *pos += len;
}

// Public: List to String
Value ds_list_to_string(Value list_val) {
  static char buffer[4096];
  int pos = 0;
  // Force type 2 (List)
  val_to_string_recursive(list_val, buffer, sizeof(buffer), &pos, 0, 2);

  char *result = (char *)gc_alloc(pos + 1, GC_TYPE_STRING);
  if (result) {
    memcpy(result, buffer, pos + 1);
    return VAL_OBJ(result);
  }
  return VAL_OBJ(buffer);
}

// Public: Object to String
Value ds_object_to_string(Value obj_val) {
  static char buffer[4096];
  int pos = 0;
  // Force type 1 (Object)
  val_to_string_recursive(obj_val, buffer, sizeof(buffer), &pos, 0, 1);

  char *result = (char *)gc_alloc(pos + 1, GC_TYPE_STRING);
  if (result) {
    memcpy(result, buffer, pos + 1);
    return VAL_OBJ(result);
  }
  return VAL_OBJ(buffer);
}

// Public: Auto to String
Value ds_val_to_string(Value val) {
  static char buffer[4096];
  int pos = 0;
  // Force type 0 (Auto)
  val_to_string_recursive(val, buffer, sizeof(buffer), &pos, 0, 0);

  char *result = (char *)gc_alloc(pos + 1, GC_TYPE_STRING);
  if (result) {
    memcpy(result, buffer, pos + 1);
    return VAL_OBJ(result);
  }
  return VAL_OBJ(buffer);
}

// Public: JSON Encode (Smart Auto - Try Object, Try List)
Value ds_json_encode(Value val) {
  static char buffer[4096];
  int pos = 0;
  // Force type 3 (Smart Auto)
  val_to_string_recursive(val, buffer, sizeof(buffer), &pos, 0, 3);

  char *result = (char *)gc_alloc(pos + 1, GC_TYPE_STRING);
  if (result) {
    memcpy(result, buffer, pos + 1);
    return VAL_OBJ(result);
  }
  return VAL_OBJ(buffer);
}

// ============================================================================
// GL Context
// ============================================================================

static int gl_initialized = 0;

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

static void ensure_gl_context(void) {
#ifdef __EMSCRIPTEN__
  if (!gl_initialized) {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.alpha = 0;
    attrs.depth = 1;
    attrs.stencil = 0;
    attrs.antialias = 1;
    attrs.premultipliedAlpha = 1;
    attrs.preserveDrawingBuffer = 0;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attrs.failIfMajorPerformanceCaveat = 0;
    attrs.enableExtensionsByDefault = 1;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx =
        emscripten_webgl_create_context("#canvas", &attrs);
    if (ctx <= 0) {
      printf("Failed to create WebGL2 context: %ld\n", (long)ctx);
      return;
    }
    emscripten_webgl_make_context_current(ctx);
    gl_initialized = 1;
  }
#else
  if (!gl_initialized) {
    int argc = 1;
    char *argv[] = {"dungeon", NULL};
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Dungeon");
    gl_initialized = 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
  }
#endif
}

// ============================================================================
// GL Bindings
// ============================================================================

void gl_color(Value r, Value g, Value b, Value a) {
  ensure_gl_context();
  glColor4f(AS_INT(r) / 255.0f, AS_INT(g) / 255.0f, AS_INT(b) / 255.0f,
            AS_INT(a) / 255.0f);
}

void gl_begin(Value mode) {
  ensure_gl_context();
  glBegin((GLenum)AS_INT(mode));
}

void gl_end(void) {
  ensure_gl_context();
  glEnd();
}

void gl_vertex(Value x, Value y) {
  ensure_gl_context();
  glVertex2f((float)AS_INT(x), (float)AS_INT(y));
}

void gl_flush(void) {
  if (gl_initialized) {
    glFlush();
  }
}

Value gl_create_shader(Value type) {
  ensure_gl_context();
  return VAL_INT((long)glCreateShader((GLenum)AS_INT(type)));
}

Value gl_shader_source_compile(Value shader, Value source) {
  ensure_gl_context();
  const char *src = (const char *)AS_OBJ(source);
  glShaderSource((GLuint)AS_INT(shader), 1, &src, NULL);
  glCompileShader((GLuint)AS_INT(shader));

  GLint success;
  glGetShaderiv((GLuint)AS_INT(shader), GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog((GLuint)AS_INT(shader), 512, NULL, infoLog);
    printf("Shader compilation failed: %s\n", infoLog);
    return VAL_INT(0);
  }
  return VAL_INT(1);
}

Value gl_create_program(void) {
  ensure_gl_context();
  return VAL_INT((long)glCreateProgram());
}

void gl_attach_shader(Value program, Value shader) {
  ensure_gl_context();
  glAttachShader((GLuint)AS_INT(program), (GLuint)AS_INT(shader));
}

Value gl_link_program(Value program) {
  ensure_gl_context();
  glLinkProgram((GLuint)AS_INT(program));

  GLint success;
  glGetProgramiv((GLuint)AS_INT(program), GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog((GLuint)AS_INT(program), 512, NULL, infoLog);
    printf("Program linking failed: %s\n", infoLog);
    return VAL_INT(0);
  }
  return VAL_INT(1);
}

void gl_use_program(Value program) {
  ensure_gl_context();
  glUseProgram((GLuint)AS_INT(program));
}

void gl_delete_shader(Value shader) {
  ensure_gl_context();
  glDeleteShader((GLuint)AS_INT(shader));
}

Value gl_get_uniform_location(Value program, Value name) {
  ensure_gl_context();
  return VAL_INT((long)glGetUniformLocation((GLuint)AS_INT(program),
                                            (const char *)AS_OBJ(name)));
}

void gl_uniform1i(Value location, Value v0) {
  ensure_gl_context();
  glUniform1i((GLint)AS_INT(location), (GLint)AS_INT(v0));
}

void gl_uniform1f(Value location, float v0) {
  ensure_gl_context();
  glUniform1f((GLint)AS_INT(location), v0);
}

void gl_uniform2f(Value location, float v0, float v1) {
  ensure_gl_context();
  glUniform2f((GLint)AS_INT(location), v0, v1);
}

void gl_uniform3f(Value location, float v0, float v1, float v2) {
  ensure_gl_context();
  glUniform3f((GLint)AS_INT(location), v0, v1, v2);
}

void gl_uniform4f(Value location, float v0, float v1, float v2, float v3) {
  ensure_gl_context();
  glUniform4f((GLint)AS_INT(location), v0, v1, v2, v3);
}

Value gl_create_buffer(void) {
  ensure_gl_context();
  GLuint buffer;
  glGenBuffers(1, &buffer);
  return VAL_INT((long)buffer);
}

void gl_bind_buffer(Value target, Value buffer) {
  ensure_gl_context();
  glBindBuffer((GLenum)AS_INT(target), (GLuint)AS_INT(buffer));
}

void gl_delete_buffer(Value buffer) {
  ensure_gl_context();
  GLuint buf = (GLuint)AS_INT(buffer);
  glDeleteBuffers(1, &buf);
}

Value gl_create_vertex_array(void) {
  ensure_gl_context();
  GLuint vao;
  glGenVertexArrays(1, &vao);
  return VAL_INT((long)vao);
}

void gl_bind_vertex_array(Value vao) {
  ensure_gl_context();
  glBindVertexArray((GLuint)AS_INT(vao));
}

void gl_delete_vertex_array(Value vao) {
  ensure_gl_context();
  GLuint v = (GLuint)AS_INT(vao);
  glDeleteVertexArrays(1, &v);
}

Value gl_get_attrib_location(Value program, Value name) {
  ensure_gl_context();
  return VAL_INT((long)glGetAttribLocation((GLuint)AS_INT(program),
                                           (const char *)AS_OBJ(name)));
}

void gl_enable_vertex_attrib_array(Value index) {
  ensure_gl_context();
  glEnableVertexAttribArray((GLuint)AS_INT(index));
}

void gl_disable_vertex_attrib_array(Value index) {
  ensure_gl_context();
  glDisableVertexAttribArray((GLuint)AS_INT(index));
}

void gl_vertex_attrib_pointer(Value index, Value size, Value type,
                              Value normalized, Value stride, Value offset) {
  ensure_gl_context();
  glVertexAttribPointer((GLuint)AS_INT(index), (GLint)AS_INT(size),
                        (GLenum)AS_INT(type), (GLboolean)AS_INT(normalized),
                        (GLsizei)AS_INT(stride), (const void *)AS_INT(offset));
}

void gl_clear(Value mask) {
  ensure_gl_context();
  glClear((GLbitfield)AS_INT(mask));
}

void gl_clear_color(float r, float g, float b, float a) {
  ensure_gl_context();
  glClearColor(r, g, b, a);
}

void gl_viewport(Value x, Value y, Value width, Value height) {
  ensure_gl_context();
  glViewport((GLint)AS_INT(x), (GLint)AS_INT(y), (GLsizei)AS_INT(width),
             (GLsizei)AS_INT(height));
}

void gl_enable(Value cap) {
  ensure_gl_context();
  glEnable((GLenum)AS_INT(cap));
}

void gl_disable(Value cap) {
  ensure_gl_context();
  glDisable((GLenum)AS_INT(cap));
}

void gl_blend_func(Value sfactor, Value dfactor) {
  ensure_gl_context();
  glBlendFunc((GLenum)AS_INT(sfactor), (GLenum)AS_INT(dfactor));
}

// Scissor functions for clipping
void gl_scissor(Value x, Value y, Value width, Value height) {
#ifdef __EMSCRIPTEN__
  glScissor(AS_INT(x), AS_INT(y), AS_INT(width), AS_INT(height));
#endif
}

// Canvas 2D clipping - save context and set clip rect
void set_clip_rect(Value x, Value y, Value w, Value h) {
#ifdef __EMSCRIPTEN__
  EM_ASM(
      {
        if (window.textCtx) {
          window.textCtx.save();
          window.textCtx.beginPath();
          window.textCtx.rect($0, $1, $2, $3);
          window.textCtx.clip();
        }
      },
      AS_INT(x), AS_INT(y), AS_INT(w), AS_INT(h));
#else
  (void)x;
  (void)y;
  (void)w;
  (void)h;
#endif
}

// Canvas 2D clipping - restore context
void clear_clip_rect(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    if (window.textCtx) {
      window.textCtx.restore();
    }
  });
#endif
}

void gl_draw_arrays(Value mode, Value first, Value count) {
  ensure_gl_context();
  glDrawArrays((GLenum)AS_INT(mode), (GLint)AS_INT(first),
               (GLsizei)AS_INT(count));
}

void gl_draw_elements(Value mode, Value count, Value type, Value offset) {
  ensure_gl_context();
  glDrawElements((GLenum)AS_INT(mode), (GLsizei)AS_INT(count),
                 (GLenum)AS_INT(type), (const void *)AS_INT(offset));
}

// ============================================================================
// Float Buffers
// ============================================================================

#define MAX_FLOAT_BUFFERS 64
#define MAX_BUFFER_SIZE 65536

typedef struct {
  float *data;
  int count;
  int in_use;
} FloatBuffer;

static FloatBuffer float_buffers[MAX_FLOAT_BUFFERS];

Value buf_create_floats(Value count) {
  long c = AS_INT(count);
  if (c <= 0 || c > MAX_BUFFER_SIZE)
    return VAL_INT(0);

  for (int i = 1; i < MAX_FLOAT_BUFFERS; i++) {
    if (!float_buffers[i].in_use) {
      float_buffers[i].in_use = 1;
      float_buffers[i].count = c;
      float_buffers[i].data =
          (float *)gc_alloc(c * sizeof(float), GC_TYPE_FLOAT_BUFFER);
      return VAL_INT(i);
    }
  }
  return VAL_INT(0);
}

void buf_set_float(Value buffer, Value index, float value) {
  long buf = AS_INT(buffer);
  long idx = AS_INT(index);
  if (buf <= 0 || buf >= MAX_FLOAT_BUFFERS)
    return;
  if (!float_buffers[buf].in_use)
    return;
  if (idx < 0 || idx >= float_buffers[buf].count)
    return;

  float_buffers[buf].data[idx] = value;
}

void buf_upload(Value target, Value buffer_handle, Value usage) {
  long buf = AS_INT(buffer_handle);
  if (buf <= 0 || buf >= MAX_FLOAT_BUFFERS)
    return;
  if (!float_buffers[buf].in_use)
    return;

  ensure_gl_context();
  glBufferData((GLenum)AS_INT(target), float_buffers[buf].count * sizeof(float),
               float_buffers[buf].data, (GLenum)AS_INT(usage));
}

void buf_free(Value buffer_handle) {
  long buf = AS_INT(buffer_handle);
  if (buf <= 0 || buf >= MAX_FLOAT_BUFFERS)
    return;
  if (!float_buffers[buf].in_use)
    return;

  // Unregister from GC and free
  gc_unregister(float_buffers[buf].data);
  free(float_buffers[buf].data);
  float_buffers[buf].in_use = 0;
}

// ============================================================================
// GC Mark and Sweep Implementation
// (Defined here after all data structures are declared)
// ============================================================================

// Recursive marker: handles both pointers (strings) and integer handles
// (Objects/Lists) This is "Conservative GC" effectively, as we treat any
// integer that *looks* like a handle as one.
static void gc_mark_value(Value val) {
  if (IS_OBJ(val)) {
    // Pointer type (String)
    gc_mark_ptr((void *)AS_OBJ(val));
  } else {
    // Integer type - could be a Masked Object handle or Masked List handle
    long id = AS_INT(val);

    // Check for Object Mask
    if ((id & TYPE_MASK_OBJ) == TYPE_MASK_OBJ) {
      long idx = id & ~TYPE_MASK_OBJ;
      if (idx > 0 && idx < MAX_OBJECTS && objects[idx].in_use &&
          !objects[idx].marked) {
        objects[idx].marked = 1;
        // Recurse
        for (int i = 0; i < objects[idx].prop_count; i++) {
          gc_mark_value(objects[idx].props[i].value);
          if (objects[idx].props[i].key) {
            gc_mark_ptr(objects[idx].props[i].key);
          }
        }
      }
    }
    // Check for List Mask
    else if ((id & TYPE_MASK_LIST) == TYPE_MASK_LIST) {
      long idx = id & ~TYPE_MASK_LIST;
      if (idx > 0 && idx < MAX_LISTS && lists[idx].in_use &&
          !lists[idx].marked) {
        lists[idx].marked = 1;
        if (lists[idx].items)
          gc_mark_ptr(lists[idx].items);
        for (int i = 0; i < lists[idx].count; i++) {
          gc_mark_value(lists[idx].items[i]);
        }
      }
    }
    // Else Plain Integer - Do Nothing
  }
}

static void gc_mark(void) {
  // Mark collected roots

  // 1. Registered root arrays (globals)
  for (int i = 0; i < GC_MAX_ROOT_ARRAYS; i++) {
    if (gc_root_arrays[i].in_use) {
      for (int j = 0; j < gc_root_arrays[i].size; j++) {
        gc_mark_value(gc_root_arrays[i].array[j]);
      }
    }
  }

  // 2. Registered root values (single globals)
  for (int i = 0; i < gc_root_value_count; i++) {
    if (gc_root_values[i]) {
      Value val = *gc_root_values[i];
      long id = AS_INT(val);
      gc_mark_value(val);
    }
  }

  // 3. Mark float buffer data
  for (int i = 0; i < MAX_FLOAT_BUFFERS; i++) {
    if (float_buffers[i].in_use) {
      gc_mark_ptr(float_buffers[i].data);
    }
  }

  // 4. Mark execution stack (temporary environments during function calls)
  for (int i = 0; i < gc_exec_stack_depth; i++) {
    gc_mark_value(gc_exec_stack[i]);
  }
}

static void gc_sweep(void) {
  // 1. Sweep Allocations (Strings, Backing Arrays)
  for (int i = 0; i < GC_MAX_ALLOCATIONS; i++) {
    if (gc_allocations[i].in_use) {
      if (!gc_allocations[i].marked) {
        // Free unmarked allocation
        gc_hash_remove(gc_allocations[i].ptr); // Remove from hash table
        free(gc_allocations[i].ptr);
        gc_allocations[i].ptr = NULL;
        gc_allocations[i].in_use = 0;
        gc_allocation_count--;
      } else {
        // Clear mark for next cycle
        gc_allocations[i].marked = 0;
      }
    }
  }

  // 2. Sweep Objects
  for (int i = 1; i < MAX_OBJECTS; i++) {
    if (objects[i].in_use) {
      if (!objects[i].marked) {
        objects[i].in_use = 0; // Reclaim slot
        // Keys are GC_TYPE_STRING and will be collected by sweep above
      } else {
        objects[i].marked = 0;
      }
    }
  }

  // 3. Sweep Lists
  for (int i = 1; i < MAX_LISTS; i++) {
    if (lists[i].in_use) {
      if (!lists[i].marked) {
        lists[i].in_use = 0; // Reclaim slot
        // items array is GC_TYPE_LIST_ITEMS and will be collected by sweep
        // above
      } else {
        lists[i].marked = 0;
      }
    }
  }
}

// ============================================================================
// Textures
// ============================================================================

Value gl_create_texture(void) {
  ensure_gl_context();
  GLuint tex;
  glGenTextures(1, &tex);
  return VAL_INT((long)tex);
}

void gl_bind_texture(Value target, Value texture) {
  ensure_gl_context();
  glBindTexture((GLenum)AS_INT(target), (GLuint)AS_INT(texture));
}

void gl_tex_parameteri(Value target, Value pname, Value param) {
  ensure_gl_context();
  glTexParameteri((GLenum)AS_INT(target), (GLenum)AS_INT(pname),
                  (GLint)AS_INT(param));
}

// ============================================================================
// Input
// ============================================================================

static int keys[512];
static int keys_prev[512];
// Accumulates keydown events between frames, then moves to _active at frame
// start
static int keys_just_pressed_pending[512];
static int keys_just_pressed_active[512]; // Read during the frame
static int shift_held = 0;

Value input_key_pressed(Value key) {
  long k = AS_INT(key);
  if (k < 0 || k >= 512)
    return VAL_INT(0);
  return VAL_INT(keys[k]);
}

Value input_key_just_pressed(Value key) {
  long k = AS_INT(key);
  if (k < 0 || k >= 512)
    return VAL_INT(0);
  // Read from active buffer - this contains events from BEFORE this frame
  // started Multiple reads per frame all return the same value
  return VAL_INT(keys_just_pressed_active[k]);
}

Value input_shift_held(void) { return VAL_INT(shift_held); }

void on_shift_down(void) { shift_held = 1; }

void on_shift_up(void) { shift_held = 0; }

// ============================================================================
// Character Input Buffer (keyboard-layout-independent text input)
// ============================================================================

#define CHAR_INPUT_BUFFER_SIZE 64
static int char_input_buffer[CHAR_INPUT_BUFFER_SIZE];
static int char_input_read_pos = 0;
static int char_input_write_pos = 0;

// Called from JS to add a character to the input buffer
void on_char_input(Value char_code) {
  int next_write = (char_input_write_pos + 1) % CHAR_INPUT_BUFFER_SIZE;
  if (next_write != char_input_read_pos) { // Buffer not full
    char_input_buffer[char_input_write_pos] = AS_INT(char_code);
    char_input_write_pos = next_write;
  }
}

// Get and consume the next character from the buffer, or 0 if empty
Value input_get_char(void) {
  if (char_input_read_pos == char_input_write_pos) {
    return VAL_INT(0); // Buffer empty
  }
  int ch = char_input_buffer[char_input_read_pos];
  char_input_read_pos = (char_input_read_pos + 1) % CHAR_INPUT_BUFFER_SIZE;
  return VAL_INT(ch);
}

// Check if there are characters available without consuming them
Value input_has_char(void) {
  return VAL_INT(char_input_read_pos != char_input_write_pos ? 1 : 0);
}

// ============================================================================
// Mouse Input
// ============================================================================

Value input_mouse_x(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.mouseX || 0; }));
#else
  return VAL_INT(0);
#endif
}

Value input_mouse_y(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.mouseY || 0; }));
#else
  return VAL_INT(0);
#endif
}

Value input_mouse_down(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.mouseDown || 0; }));
#else
  return VAL_INT(0);
#endif
}

Value input_mouse_just_pressed(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.mouseJustPressed || 0; }));
#else
  return VAL_INT(0);
#endif
}

Value input_mouse_just_released(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.mouseJustReleased || 0; }));
#else
  return VAL_INT(0);
#endif
}

// ============================================================================
// Scroll Wheel Input
// ============================================================================

Value input_scroll_delta(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.scrollDeltaY || 0; }));
#else
  return VAL_INT(0);
#endif
}

void input_scroll_clear(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM({ window.scrollDeltaY = 0; });
#endif
}

// ============================================================================
// Clipboard
// ============================================================================

Value clipboard_copy_requested(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.clipboardCopyRequested || 0; }));
#else
  return VAL_INT(0);
#endif
}

Value clipboard_paste_requested(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.clipboardPasteRequested || 0; }));
#else
  return VAL_INT(0);
#endif
}

static char clipboard_buffer[16384];

Value clipboard_get_text(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM(
      {
        var text = window.clipboardText || "";
        var ptr = $0;
        for (var i = 0; i < text.length && i < 16383; i++) {
          HEAP8[ptr + i] = text.charCodeAt(i);
        }
        HEAP8[ptr + Math.min(text.length, 16383)] = 0;
      },
      clipboard_buffer);
  return VAL_OBJ(clipboard_buffer);
#else
  clipboard_buffer[0] = 0;
  return VAL_OBJ(clipboard_buffer);
#endif
}

void clipboard_set_text(Value str) {
#ifdef __EMSCRIPTEN__
  const char *s = (const char *)AS_OBJ(str);
  EM_ASM(
      {
        var str = UTF8ToString($0);
        window.clipboardText = str;
        navigator.clipboard.writeText(str).catch(function(){});
      },
      s);
#else
  (void)str;
#endif
}

void clipboard_clear_requests(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    window.clipboardCopyRequested = 0;
    window.clipboardPasteRequested = 0;
    window.selectAllRequested = 0;
  });
#endif
}

Value select_all_requested(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.selectAllRequested || 0; }));
#else
  return VAL_INT(0);
#endif
}

// ============================================================================
// Console / Debug
// ============================================================================

void console_log(Value msg) {
  const char *str = (const char *)AS_OBJ(msg);
  printf("%s\n", str ? str : "(null)");
  fflush(stdout);
}

void console_log_int(Value value) { printf("%ld\n", AS_INT(value)); }

void console_log_float(float value) { printf("%f\n", value); }

// ============================================================================
// Text Rendering
// ============================================================================

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Screen dimensions - can be updated dynamically
static int screen_width = 1280;
static int screen_height = 720;

Value get_screen_width(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.screenWidth || 1280; }));
#else
  return VAL_INT(screen_width);
#endif
}

Value get_screen_height(void) {
#ifdef __EMSCRIPTEN__
  return VAL_INT(EM_ASM_INT({ return window.screenHeight || 720; }));
#else
  return VAL_INT(screen_height);
#endif
}

void set_skip_crt(Value skip) {
#ifdef __EMSCRIPTEN__
  EM_ASM_({ window.skipCRT = $0; }, AS_INT(skip));
#else
  (void)skip;
#endif
}

void text_clear(void) {
#ifdef __EMSCRIPTEN__
  int w = AS_INT(get_screen_width());
  int h = AS_INT(get_screen_height());
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.clearRect(0, 0, $0, $1);
          window.textCtx.font = '14px "Berkeley Mono", monospace';
          window.textCtx.textBaseline = 'top';
        }
      },
      w, h);
#else
  // Native GL clear handles it
#endif
}

void text_draw(Value x, Value y, Value size, Value r, Value g, Value b,
               Value text_val) {
  const char *text = (const char *)AS_OBJ(text_val);
  int fontSize = (int)AS_INT(size);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.font = $6 + 'px "Berkeley Mono", monospace';
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(UTF8ToString($3), $4, $5);
        }
      },
      AS_INT(r), AS_INT(g), AS_INT(b), text, AS_INT(x), AS_INT(y), fontSize);
#else
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor3f(AS_INT(r) / 255.0f, AS_INT(g) / 255.0f, AS_INT(b) / 255.0f);
  glRasterPos2i(AS_INT(x), SCREEN_HEIGHT - AS_INT(y));

  while (text && *text) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text);
    text++;
  }
  glPopAttrib();
#endif
}

void text_char(Value x, Value y, Value size, Value r, Value g, Value b,
               Value c) {
  int ch = (int)AS_INT(c); // Unwrap tagged character code
  int fontSize = (int)AS_INT(size);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.font = $6 + 'px "Berkeley Mono", monospace';
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(String.fromCharCode($3), $4, $5);
        }
      },
      AS_INT(r), AS_INT(g), AS_INT(b), ch, AS_INT(x), AS_INT(y), fontSize);
#else
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor3f(AS_INT(r) / 255.0f, AS_INT(g) / 255.0f, AS_INT(b) / 255.0f);
  glRasterPos2i(AS_INT(x), SCREEN_HEIGHT - AS_INT(y));

  glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ch);
  glPopAttrib();
#endif
}

void text_draw_int(Value x, Value y, Value size, Value r, Value g, Value b,
                   Value value) {
  char buf[32];
  sprintf(buf, "%ld", AS_INT(value));
  text_draw(x, y, size, r, g, b, VAL_OBJ(buf));
}

// Draw text with a custom font (for intro/special screens)
// font_id: 0 = serif (Times/Georgia), 1 = handwriting (cursive), 2 = typewriter
// (Courier)
void text_draw_font(Value x, Value y, Value size, Value r, Value g, Value b,
                    Value font_id, Value text_val) {
  const char *text = (const char *)AS_OBJ(text_val);
  int font = (int)AS_INT(font_id);
  int sz = (int)AS_INT(size);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          var fontFamily;
          switch ($6) {
          case 0:
            fontFamily = 'Georgia, "Times New Roman", serif';
            break;
          case 1:
            fontFamily = '"Brush Script MT", "Segoe Script", cursive';
            break;
          case 2:
            fontFamily = '"Courier New", Courier, monospace';
            break;
          default:
            fontFamily = 'Georgia, serif';
          }
          window.textCtx.font = $7 + 'px ' + fontFamily;
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(UTF8ToString($3), $4, $5);
          // Reset to game font
          window.textCtx.font = '14px "Berkeley Mono", monospace';
        }
      },
      AS_INT(r), AS_INT(g), AS_INT(b), text, AS_INT(x), AS_INT(y), font, sz);
#else
  text_draw(x, y, size, r, g, b, text_val);
#endif
}

void text_draw_font_right(Value x, Value y, Value size, Value r, Value g,
                          Value b, Value font_id, Value text_val) {
  const char *text = (const char *)AS_OBJ(text_val);
  int font = (int)AS_INT(font_id);
  int sz = (int)AS_INT(size);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          var fontFamily;
          switch ($6) {
          case 0:
            fontFamily = 'Georgia, "Times New Roman", serif';
            break;
          case 1:
            fontFamily = '"Brush Script MT", "Segoe Script", cursive';
            break;
          case 2:
            fontFamily = '"Courier New", Courier, monospace';
            break;
          default:
            fontFamily = 'Georgia, serif';
          }
          window.textCtx.font = $7 + 'px ' + fontFamily;
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          var textStr = UTF8ToString($3);
          var textWidth = window.textCtx.measureText(textStr).width;
          window.textCtx.fillText(textStr, $4 - textWidth, $5);
          // Reset to game font
          window.textCtx.font = '14px "Berkeley Mono", monospace';
        }
      },
      AS_INT(r), AS_INT(g), AS_INT(b), text, AS_INT(x), AS_INT(y), font, sz);
#else
  text_draw(x, y, size, r, g, b, text_val);
#endif
}

// Measure text width in pixels (for monospace Berkeley Mono font)
Value text_measure(Value text_val, Value font_size) {
  const char *text = (const char *)AS_OBJ(text_val);
  int sz = (int)AS_INT(font_size);
  // For monospace fonts, character width is approximately 0.6 * font_size
  int len = (int)strlen(text);
  int width = (len * sz * 6) / 10;
  return VAL_INT(width);
}

void draw_rect(Value x, Value y, Value w, Value h, Value r, Value g, Value b,
               Value a) {
#ifdef __EMSCRIPTEN__
  EM_ASM(
      {
        if (window.textCtx) {
          window.textCtx.fillStyle =
              'rgba(' + $4 + ',' + $5 + ',' + $6 + ',' + ($7 / 255.0) + ')';
          window.textCtx.fillRect($0, $1, $2, $3);
        }
      },
      AS_INT(x), AS_INT(y), AS_INT(w), AS_INT(h), AS_INT(r), AS_INT(g),
      AS_INT(b), AS_INT(a));
#else
  (void)x;
  (void)y;
  (void)w;
  (void)h;
  (void)r;
  (void)g;
  (void)b;
  (void)a;
#endif
}

// Draw a line segment (for signature drawing)
void draw_line(Value x1, Value y1, Value x2, Value y2, Value width, Value r,
               Value g, Value b, Value a) {
#ifdef __EMSCRIPTEN__
  EM_ASM(
      {
        if (window.textCtx) {
          window.textCtx.strokeStyle =
              'rgba(' + $5 + ',' + $6 + ',' + $7 + ',' + ($8 / 255.0) + ')';
          window.textCtx.lineWidth = $4;
          window.textCtx.lineCap = 'round';
          window.textCtx.lineJoin = 'round';
          window.textCtx.beginPath();
          window.textCtx.moveTo($0, $1);
          window.textCtx.lineTo($2, $3);
          window.textCtx.stroke();
        }
      },
      AS_INT(x1), AS_INT(y1), AS_INT(x2), AS_INT(y2), AS_INT(width), AS_INT(r),
      AS_INT(g), AS_INT(b), AS_INT(a));
#else
  (void)x1;
  (void)y1;
  (void)x2;
  (void)y2;
  (void)width;
  (void)r;
  (void)g;
  (void)b;
  (void)a;
#endif
}

// ============================================================================
// Math
// ============================================================================

Value math_sin(Value x) {
  return VAL_INT((long)(sin((double)AS_INT(x)) * 1000));
}

Value math_cos(Value x) {
  return VAL_INT((long)(cos((double)AS_INT(x)) * 1000));
}

Value math_random(Value min, Value max) {
  long mn = AS_INT(min);
  long mx = AS_INT(max);
  if (mx <= mn)
    return VAL_INT(mn);
  return VAL_INT(mn + rand() % (mx - mn + 1));
}

float math_sqrt(float x) { return sqrtf(x); }
float math_floor(float x) { return floorf(x); }
float math_ceil(float x) { return ceilf(x); }
float math_abs(float x) { return fabsf(x); }
float math_min(float a, float b) { return a < b ? a : b; }
float math_max(float a, float b) { return a > b ? a : b; }

// ============================================================================
// RNG - xorshift32 (better quality than stdlib rand)
// ============================================================================

static uint32_t rng_state = 1;

void rng_seed(Value seed) {
  rng_state = (uint32_t)AS_INT(seed);
  if (rng_state == 0)
    rng_state = 1; // State must be non-zero
}

Value rng_int(Value max) {
  long mx = AS_INT(max);
  if (mx <= 0)
    return VAL_INT(0);

  // xorshift32
  uint32_t x = rng_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;

  return VAL_INT(x % (uint32_t)mx);
}

float rng_float(void) {
  // Generate a random number first
  uint32_t x = rng_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;

  return (float)x / (float)UINT32_MAX;
}

// ============================================================================
// Game Loop Exports
// ============================================================================

#ifndef GAME_BUILD
void game_init(void) {}
void game_update(Value dt) { (void)dt; }
void game_render(void) {}
#endif

void on_key_down(Value key) {
  long k = AS_INT(key);
  if (k >= 0 && k < 512) {
    keys[k] = 1;
    keys_just_pressed_pending[k] = 1; // Accumulate for next frame
  }
}

void on_key_up(Value key) {
  long k = AS_INT(key);
  if (k >= 0 && k < 512)
    keys[k] = 0;
}

void on_frame_start(void) {
  memcpy(keys_prev, keys, sizeof(keys));
  // Transfer pending just_pressed events to active buffer for this frame
  // This ensures keys pressed BETWEEN frames are detected this frame
  memcpy(keys_just_pressed_active, keys_just_pressed_pending,
         sizeof(keys_just_pressed_active));
  memset(keys_just_pressed_pending, 0, sizeof(keys_just_pressed_pending));
  // GC collection is now safe because __gc_register_roots() registers
  // all global arrays and string variables as roots at game_init
  gc_maybe_collect();
}

// ============================================================================
// Audio
// ============================================================================

void play_sound(Value id_val) {
  int id = (int)AS_INT(id_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.playGameSound)
          window.playGameSound($0);
      },
      id);
#else
  (void)id;
#endif
}

// ============================================================================
// Volume Control
// ============================================================================

void js_call_set_volume(Value level_val) {
  int level = (int)AS_INT(level_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.setGameVolume)
          window.setGameVolume($0);
      },
      level);
#else
  (void)level;
#endif
}

void js_call_set_master_volume(Value level_val) {
  int level = (int)AS_INT(level_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.setMasterVolume)
          window.setMasterVolume($0);
      },
      level);
#else
  (void)level;
#endif
}

void js_call_set_music_volume(Value level_val) {
  int level = (int)AS_INT(level_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.setMusicVolume)
          window.setMusicVolume($0);
      },
      level);
#else
  (void)level;
#endif
}

void js_call_set_sfx_volume(Value level_val) {
  int level = (int)AS_INT(level_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.setSfxVolume)
          window.setSfxVolume($0);
      },
      level);
#else
  (void)level;
#endif
}

void js_call_muffle_music(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    if (window.muffleMusic)
      window.muffleMusic();
  });
#endif
}

void js_call_unmuffle_music(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    if (window.unmuffleMusic)
      window.unmuffleMusic();
  });
#endif
}

// ============================================================================
// URL Opening (opens in new tab)
// ============================================================================

void open_url(Value url_val) {
  const char *url = (const char *)AS_OBJ(url_val);
  if (!url)
    return;
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        var url = UTF8ToString($0);
        window.open(url, '_blank');
      },
      url);
#else
  (void)url;
#endif
}

// ============================================================================
// Save/Load System
// ============================================================================

void js_call_save_game(Value slot_val, Value code_val, Value upgrades_val,
                       Value chat_state_val) {
  int slot = (int)AS_INT(slot_val);
  const char *code = (const char *)AS_OBJ(code_val);
  const char *upgrades = (const char *)AS_OBJ(upgrades_val);
  const char *chat_state = (const char *)AS_OBJ(chat_state_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.saveGameData) {
          var code = UTF8ToString($1);
          var upgrades = UTF8ToString($2);
          var chat_state = UTF8ToString($3);
          window.saveGameData($0, code, upgrades, chat_state);
        }
      },
      slot, code, upgrades, chat_state);
#else
  (void)slot;
  (void)code;
  (void)upgrades;
  (void)chat_state;
#endif
}

static char load_buffer[65536];

Value js_call_load_game(Value slot_val) {
  int slot = (int)AS_INT(slot_val);
#ifdef __EMSCRIPTEN__
  int result = EM_ASM_INT(
      {
        var data = window.loadGameData ? window.loadGameData($0) : null;
        if (!data)
          return 0;
        var ptr = $1;
        for (var i = 0; i < data.length && i < 65535; i++) {
          HEAP8[ptr + i] = data.charCodeAt(i);
        }
        HEAP8[ptr + Math.min(data.length, 65535)] = 0;
        return 1;
      },
      slot, load_buffer);

  if (result == 0)
    return VAL_INT(0);
  return VAL_OBJ(load_buffer);
#else
  (void)slot;
  return VAL_INT(0);
#endif
}

Value js_call_save_exists(Value slot_val) {
  int slot = (int)AS_INT(slot_val);
#ifdef __EMSCRIPTEN__
  int exists = EM_ASM_INT(
      { return window.saveSlotExists ? window.saveSlotExists($0) : 0; }, slot);
  return VAL_INT(exists);
#else
  (void)slot;
  return VAL_INT(0);
#endif
}

// ============================================================================
// Settings Persistence
// ============================================================================

static char setting_buffer[256];

Value js_call_get_setting(Value key_val) {
  const char *key = (const char *)AS_OBJ(key_val);
#ifdef __EMSCRIPTEN__
  int val = EM_ASM_INT(
      { return window.getSetting ? window.getSetting(UTF8ToString($0)) : 0; },
      key);
  return VAL_INT(val);
#else
  (void)key;
  return VAL_INT(0);
#endif
}

void js_call_set_setting(Value key_val, Value value_val) {
  const char *key = (const char *)AS_OBJ(key_val);
  int value = (int)AS_INT(value_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.setSetting)
          window.setSetting(UTF8ToString($0), $1);
      },
      key, value);
#else
  (void)key;
  (void)value;
#endif
}

// ============================================================================
// JSON Parsing Helpers (for Save Data)
// ============================================================================

static char json_code_buffer[65536];
static char json_upgrades_buffer[1024];

Value js_parse_save_code(Value json_val) {
  const char *json = (const char *)AS_OBJ(json_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        try {
          var obj = JSON.parse(UTF8ToString($0));
          var code = obj.code || "";
          var ptr = $1;
          for (var i = 0; i < code.length && i < 65535; i++) {
    HEAP8[ptr + i] = code.charCodeAt(i);
          }
          HEAP8[ptr + Math.min(code.length, 65535)] = 0;
}
catch(e) { HEAP8[$1] = 0; }
},
      json, json_code_buffer);
return VAL_OBJ(json_code_buffer);
#else
  (void)json;
  json_code_buffer[0] = 0;
  return VAL_OBJ(json_code_buffer);
#endif
}

Value js_parse_save_upgrades(Value json_val) {
  const char *json = (const char *)AS_OBJ(json_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        try {
          var obj = JSON.parse(UTF8ToString($0));
          var upgrades = obj.upgrades || "";
          var ptr = $1;
          for (var i = 0; i < upgrades.length && i < 1023; i++) {
    HEAP8[ptr + i] = upgrades.charCodeAt(i);
          }
          HEAP8[ptr + Math.min(upgrades.length, 1023)] = 0;
}
catch(e) { HEAP8[$1] = 0; }
},
      json, json_upgrades_buffer);
return VAL_OBJ(json_upgrades_buffer);
#else
  (void)json;
  json_upgrades_buffer[0] = 0;
  return VAL_OBJ(json_upgrades_buffer);
#endif
}

static char json_chat_state_buffer[65536];

Value js_parse_save_chat_state(Value json_val) {
  const char *json = (const char *)AS_OBJ(json_val);
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        try {
          var obj = JSON.parse(UTF8ToString($0));
          var chat = obj.chat_state || "";
          var ptr = $1;
          for (var i = 0; i < chat.length && i < 65535; i++) {
    HEAP8[ptr + i] = chat.charCodeAt(i);
          }
          HEAP8[ptr + Math.min(chat.length, 65535)] = 0;
}
catch(e) { HEAP8[$1] = 0; }
},
      json, json_chat_state_buffer);
return VAL_OBJ(json_chat_state_buffer);
#else
  (void)json;
  json_chat_state_buffer[0] = 0;
  return VAL_OBJ(json_chat_state_buffer);
#endif
}

// Menu Art Fetching
Value js_get_menu_art_count(void) {
#ifdef __EMSCRIPTEN__
  int count = EM_ASM_INT({
    return window.getMenuArtLineCount ? window.getMenuArtLineCount() : 0;
  });
  return VAL_INT(count);
#else
  return VAL_INT(0);
#endif
}

Value js_get_menu_art_line(Value idx_val) {
  int idx = (int)AS_INT(idx_val);
#ifdef __EMSCRIPTEN__
  int result = EM_ASM_INT(
      {
        var line = window.getMenuArtLine ? window.getMenuArtLine($0) : "";
        var ptr = $1;
        // Copy string to heap
        for (var i = 0; i < line.length && i < 65535; i++) {
          HEAP8[ptr + i] = line.charCodeAt(i);
        }
        HEAP8[ptr + Math.min(line.length, 65535)] = 0;
        return line.length;
      },
      idx, load_buffer);

  return VAL_OBJ(load_buffer);
#else
  return VAL_OBJ("");
#endif
}
