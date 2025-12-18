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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static Object objects[MAX_OBJECTS];
static int objects_initialized = 0;

static void init_objects(void) {
  if (objects_initialized)
    return;
  memset(objects, 0, sizeof(objects));
  objects_initialized = 1;
}

static Value alloc_object(void) {
  init_objects();
  for (int i = 1; i < MAX_OBJECTS; i++) {
    if (!objects[i].in_use) {
      objects[i].in_use = 1;
      objects[i].prop_count = 0;
      return VAL_INT(i);
    }
  }
  return VAL_INT(0);
}

Value ds_object_create(Value count_val, ...) {
  int count = (int)AS_INT(count_val);
  Value handle = alloc_object();
  if (AS_INT(handle) == 0)
    return VAL_INT(0);

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
  long obj = AS_INT(obj_val);
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
  if (handle <= 0 || handle >= MAX_OBJECTS) {
    // Auto-allocate if null
    handle_val = alloc_object();
    handle = AS_INT(handle_val);
    *obj = handle_val;
  }

  if (!objects[handle].in_use)
    return;

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

void ds_set_prop(Value obj_val, Value key_val, Value value) {
  long obj = AS_INT(obj_val);
  if (obj <= 0 || obj >= MAX_OBJECTS)
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
  char *sub = malloc(len + 1);
  strncpy(sub, str + start, len);
  sub[len] = '\0';
  return VAL_OBJ(sub); // Leaks memory, but fine for this demo
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

  char *result = malloc(len + 2);
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

  char *result = malloc(len);
  strncpy(result, str, pos);
  strcpy(result + pos, str + pos + 1);
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
  char *result = malloc(len1 + len2 + 1);
  strcpy(result, str1);
  strcpy(result + len1, str2);
  return VAL_OBJ(result);
}

// Create a single-character string from char code
Value ds_char_to_string(Value char_code_val) {
  long char_code = AS_INT(char_code_val);
  char *result = malloc(2);
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
      lists[i].count = 0;
      lists[i].capacity = 16;
      lists[i].items = (Value *)malloc(lists[i].capacity * sizeof(Value));
      return VAL_INT(i);
    }
  }
  return VAL_INT(0);
}

Value ds_list_push(Value list_val, Value value) {
  long list = AS_INT(list_val);
  if (list <= 0 || list >= MAX_LISTS)
    return VAL_INT(0);
  if (!lists[list].in_use)
    return VAL_INT(0);

  if (lists[list].count >= lists[list].capacity) {
    lists[list].capacity *= 2;
    lists[list].items = (Value *)realloc(lists[list].items,
                                         lists[list].capacity * sizeof(Value));
  }
  lists[list].items[lists[list].count++] = value;
  return VAL_INT(0);
}

Value ds_list_get(Value list_val, Value index_val) {
  long list = AS_INT(list_val);
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
  long list = AS_INT(list_val);
  if (list <= 0 || list >= MAX_LISTS)
    return VAL_INT(0);
  if (!lists[list].in_use)
    return VAL_INT(0);
  return VAL_INT(lists[list].count);
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
      float_buffers[i].data = (float *)malloc(c * sizeof(float));
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

  free(float_buffers[buf].data);
  float_buffers[buf].in_use = 0;
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
  return VAL_INT(keys[k] && !keys_prev[k]);
}

Value input_shift_held(void) { return VAL_INT(shift_held); }

void on_shift_down(void) { shift_held = 1; }

void on_shift_up(void) { shift_held = 0; }

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

void console_log_int(Value value) { printf("[LOG] %ld\n", AS_INT(value)); }

void console_log_float(float value) { printf("[LOG] %f\n", value); }

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
  (void)size;
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(UTF8ToString($3), $4, $5);
        }
      },
      AS_INT(r), AS_INT(g), AS_INT(b), text, AS_INT(x), AS_INT(y));
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
  (void)size;
  int ch = (int)AS_INT(c); // Unwrap tagged character code
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(String.fromCharCode($3), $4, $5);
        }
      },
      AS_INT(r), AS_INT(g), AS_INT(b), ch, AS_INT(x), AS_INT(y));
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
// RNG
// ============================================================================

void rng_seed(Value seed) { srand((unsigned int)AS_INT(seed)); }

Value rng_int(Value max) {
  long mx = AS_INT(max);
  if (mx <= 0)
    return VAL_INT(0);
  return VAL_INT(rand() % mx);
}

float rng_float(void) { return (float)rand() / (float)RAND_MAX; }

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
  if (k >= 0 && k < 512)
    keys[k] = 1;
}

void on_key_up(Value key) {
  long k = AS_INT(key);
  if (k >= 0 && k < 512)
    keys[k] = 0;
}

void on_frame_start(void) { memcpy(keys_prev, keys, sizeof(keys)); }
