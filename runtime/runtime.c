#define GL_GLEXT_PROTOTYPES
#include "runtime.h"
#include <GL/glut.h>
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

void text_write_int(Value val) { printf("%ld", val); }

Value time_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (Value)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
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
      return i;
    }
  }
  return 0;
}

Value ds_object_create(int count, ...) {
  Value handle = alloc_object();
  if (handle == 0)
    return 0;

  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    const char *key = va_arg(args, const char *);
    Value val = va_arg(args, Value);
    ds_object_set(&handle, key, val);
  }

  va_end(args);
  return handle;
}

Value ds_object_get(Value obj, const char *key) {
  if (obj <= 0 || obj >= MAX_OBJECTS)
    return 0;
  if (!objects[obj].in_use)
    return 0;

  for (int i = 0; i < objects[obj].prop_count; i++) {
    if (strcmp(objects[obj].props[i].key, key) == 0) {
      return objects[obj].props[i].value;
    }
  }
  return 0;
}

void ds_object_set(Value *obj, const char *key, Value value) {
  Value handle = *obj;
  if (handle <= 0 || handle >= MAX_OBJECTS) {
    // Auto-allocate if null
    handle = alloc_object();
    *obj = handle;
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

void ds_set_prop(Value obj, Value key_val, Value value) {
  const char *key = (const char *)key_val;
  if (obj <= 0 || obj >= MAX_OBJECTS)
    return;

  // Reuse ds_object_set logic or call it?
  // We can't pass address of local variable obj to ds_object_set because it
  // might update it, but we want to update the object referred to by obj.
  // Actually ds_object_set updates the HANDLE if it was 0.
  // Here obj is already a handle.
  // So we can just cast address.
  ds_object_set(&obj, key, value);
}

Value ds_strlen(Value str_val) {
  const char *str = (const char *)str_val;
  if (!str)
    return 0;
  return (Value)strlen(str);
}
Value ds_streq(Value s1_val, Value s2_val) {
  const char *s1 = (const char *)s1_val;
  const char *s2 = (const char *)s2_val;
  if (!s1 || !s2)
    return 0;
  return strcmp(s1, s2) == 0;
}

Value ds_div(Value a, Value b) {
  if (b == 0)
    return 0;
  return a / b;
}

Value ds_mod(Value a, Value b) {
  if (b == 0)
    return 0;
  return a % b;
}
Value ds_substring(Value str_val, Value start, Value len) {
  const char *str = (const char *)str_val;
  if (!str || len <= 0)
    return 0;
  char *sub = malloc(len + 1);
  strncpy(sub, str + start, len);
  sub[len] = '\0';
  return (Value)sub; // Leaks memory, but fine for this demo
}

Value ds_string_at(Value str_val, Value index) {
  const char *str = (const char *)str_val;
  if (!str)
    return 0;
  if (index < 0 || index >= (Value)strlen(str))
    return 0;
  return (Value)str[index];
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
      return i;
    }
  }
  return 0;
}

Value ds_list_push(Value list, Value value) {
  if (list <= 0 || list >= MAX_LISTS)
    return 0;
  if (!lists[list].in_use)
    return 0;

  if (lists[list].count >= lists[list].capacity) {
    lists[list].capacity *= 2;
    lists[list].items = (Value *)realloc(lists[list].items,
                                         lists[list].capacity * sizeof(Value));
  }
  lists[list].items[lists[list].count++] = value;
  return 0;
}

Value ds_list_get(Value list, Value index) {
  if (list <= 0 || list >= MAX_LISTS)
    return 0;
  if (!lists[list].in_use)
    return 0;
  if (index < 0 || index >= lists[list].count)
    return 0;

  return lists[list].items[index];
}

Value ds_list_len(Value list) {
  if (list <= 0 || list >= MAX_LISTS)
    return 0;
  if (!lists[list].in_use)
    return 0;
  return lists[list].count;
}

// ============================================================================
// GL Context
// ============================================================================

static int gl_initialized = 0;

static void ensure_gl_context(void) {
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
}

// ============================================================================
// GL Bindings
// ============================================================================

void gl_color(Value r, Value g, Value b, Value a) {
  ensure_gl_context();
  glColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

void gl_begin(Value mode) {
  ensure_gl_context();
  glBegin((GLenum)mode);
}

void gl_end(void) {
  ensure_gl_context();
  glEnd();
}

void gl_vertex(Value x, Value y) {
  ensure_gl_context();
  glVertex2f((float)x, (float)y);
}

void gl_flush(void) {
  if (gl_initialized) {
    glFlush();
  }
}

Value gl_create_shader(Value type) {
  ensure_gl_context();
  return (Value)glCreateShader((GLenum)type);
}

Value gl_shader_source_compile(Value shader, const char *source) {
  ensure_gl_context();
  glShaderSource((GLuint)shader, 1, &source, NULL);
  glCompileShader((GLuint)shader);

  GLint success;
  glGetShaderiv((GLuint)shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog((GLuint)shader, 512, NULL, infoLog);
    printf("Shader compilation failed: %s\n", infoLog);
    return 0;
  }
  return 1;
}

Value gl_create_program(void) {
  ensure_gl_context();
  return (Value)glCreateProgram();
}

void gl_attach_shader(Value program, Value shader) {
  ensure_gl_context();
  glAttachShader((GLuint)program, (GLuint)shader);
}

Value gl_link_program(Value program) {
  ensure_gl_context();
  glLinkProgram((GLuint)program);

  GLint success;
  glGetProgramiv((GLuint)program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog((GLuint)program, 512, NULL, infoLog);
    printf("Program linking failed: %s\n", infoLog);
    return 0;
  }
  return 1;
}

void gl_use_program(Value program) {
  ensure_gl_context();
  glUseProgram((GLuint)program);
}

void gl_delete_shader(Value shader) {
  ensure_gl_context();
  glDeleteShader((GLuint)shader);
}

Value gl_get_uniform_location(Value program, const char *name) {
  ensure_gl_context();
  return (Value)glGetUniformLocation((GLuint)program, name);
}

void gl_uniform1i(Value location, Value v0) {
  ensure_gl_context();
  glUniform1i((GLint)location, (GLint)v0);
}

void gl_uniform1f(Value location, float v0) {
  ensure_gl_context();
  glUniform1f((GLint)location, v0);
}

void gl_uniform2f(Value location, float v0, float v1) {
  ensure_gl_context();
  glUniform2f((GLint)location, v0, v1);
}

void gl_uniform3f(Value location, float v0, float v1, float v2) {
  ensure_gl_context();
  glUniform3f((GLint)location, v0, v1, v2);
}

void gl_uniform4f(Value location, float v0, float v1, float v2, float v3) {
  ensure_gl_context();
  glUniform4f((GLint)location, v0, v1, v2, v3);
}

Value gl_create_buffer(void) {
  ensure_gl_context();
  GLuint buffer;
  glGenBuffers(1, &buffer);
  return (Value)buffer;
}

void gl_bind_buffer(Value target, Value buffer) {
  ensure_gl_context();
  glBindBuffer((GLenum)target, (GLuint)buffer);
}

void gl_delete_buffer(Value buffer) {
  ensure_gl_context();
  GLuint buf = (GLuint)buffer;
  glDeleteBuffers(1, &buf);
}

Value gl_create_vertex_array(void) {
  ensure_gl_context();
  GLuint vao;
  glGenVertexArrays(1, &vao);
  return (Value)vao;
}

void gl_bind_vertex_array(Value vao) {
  ensure_gl_context();
  glBindVertexArray((GLuint)vao);
}

void gl_delete_vertex_array(Value vao) {
  ensure_gl_context();
  GLuint v = (GLuint)vao;
  glDeleteVertexArrays(1, &v);
}

Value gl_get_attrib_location(Value program, const char *name) {
  ensure_gl_context();
  return (Value)glGetAttribLocation((GLuint)program, name);
}

void gl_enable_vertex_attrib_array(Value index) {
  ensure_gl_context();
  glEnableVertexAttribArray((GLuint)index);
}

void gl_disable_vertex_attrib_array(Value index) {
  ensure_gl_context();
  glDisableVertexAttribArray((GLuint)index);
}

void gl_vertex_attrib_pointer(Value index, Value size, Value type,
                              Value normalized, Value stride, Value offset) {
  ensure_gl_context();
  glVertexAttribPointer((GLuint)index, (GLint)size, (GLenum)type,
                        (GLboolean)normalized, (GLsizei)stride,
                        (const void *)offset);
}

void gl_clear(Value mask) {
  ensure_gl_context();
  glClear((GLbitfield)mask);
}

void gl_clear_color(float r, float g, float b, float a) {
  ensure_gl_context();
  glClearColor(r, g, b, a);
}

void gl_viewport(Value x, Value y, Value width, Value height) {
  ensure_gl_context();
  glViewport((GLint)x, (GLint)y, (GLsizei)width, (GLsizei)height);
}

void gl_enable(Value cap) {
  ensure_gl_context();
  glEnable((GLenum)cap);
}

void gl_disable(Value cap) {
  ensure_gl_context();
  glDisable((GLenum)cap);
}

void gl_blend_func(Value sfactor, Value dfactor) {
  ensure_gl_context();
  glBlendFunc((GLenum)sfactor, (GLenum)dfactor);
}

void gl_draw_arrays(Value mode, Value first, Value count) {
  ensure_gl_context();
  glDrawArrays((GLenum)mode, (GLint)first, (GLsizei)count);
}

void gl_draw_elements(Value mode, Value count, Value type, Value offset) {
  ensure_gl_context();
  glDrawElements((GLenum)mode, (GLsizei)count, (GLenum)type,
                 (const void *)offset);
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
  if (count <= 0 || count > MAX_BUFFER_SIZE)
    return 0;

  for (int i = 1; i < MAX_FLOAT_BUFFERS; i++) {
    if (!float_buffers[i].in_use) {
      float_buffers[i].in_use = 1;
      float_buffers[i].count = count;
      float_buffers[i].data = (float *)malloc(count * sizeof(float));
      return i;
    }
  }
  return 0;
}

void buf_set_float(Value buffer, Value index, float value) {
  if (buffer <= 0 || buffer >= MAX_FLOAT_BUFFERS)
    return;
  if (!float_buffers[buffer].in_use)
    return;
  if (index < 0 || index >= float_buffers[buffer].count)
    return;

  float_buffers[buffer].data[index] = value;
}

void buf_upload(Value target, Value buffer_handle, Value usage) {
  if (buffer_handle <= 0 || buffer_handle >= MAX_FLOAT_BUFFERS)
    return;
  if (!float_buffers[buffer_handle].in_use)
    return;

  ensure_gl_context();
  glBufferData((GLenum)target,
               float_buffers[buffer_handle].count * sizeof(float),
               float_buffers[buffer_handle].data, (GLenum)usage);
}

void buf_free(Value buffer_handle) {
  if (buffer_handle <= 0 || buffer_handle >= MAX_FLOAT_BUFFERS)
    return;
  if (!float_buffers[buffer_handle].in_use)
    return;

  free(float_buffers[buffer_handle].data);
  float_buffers[buffer_handle].in_use = 0;
}

// ============================================================================
// Textures
// ============================================================================

Value gl_create_texture(void) {
  ensure_gl_context();
  GLuint tex;
  glGenTextures(1, &tex);
  return (Value)tex;
}

void gl_bind_texture(Value target, Value texture) {
  ensure_gl_context();
  glBindTexture((GLenum)target, (GLuint)texture);
}

void gl_tex_parameteri(Value target, Value pname, Value param) {
  ensure_gl_context();
  glTexParameteri((GLenum)target, (GLenum)pname, (GLint)param);
}

// ============================================================================
// Input
// ============================================================================

static int keys[512];
static int keys_prev[512];

Value input_key_pressed(Value key) {
  if (key < 0 || key >= 512)
    return 0;
  return keys[key];
}

Value input_key_just_pressed(Value key) {
  if (key < 0 || key >= 512)
    return 0;
  return keys[key] && !keys_prev[key];
}

// ============================================================================
// Console / Debug
// ============================================================================

void console_log(const char *msg) {
  printf("%s\n", msg);
  fflush(stdout);
}

void console_log_int(Value value) { printf("[LOG] %ld\n", value); }

void console_log_float(float value) { printf("[LOG] %f\n", value); }

// ============================================================================
// Text Rendering
// ============================================================================

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void text_clear(void) {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    if (window.textCtx) {
      window.textCtx.clearRect(0, 0, 800, 600);
      window.textCtx.font = '15px "Berkeley Mono", monospace';
      window.textCtx.textBaseline = 'top';
    }
  });
#else
  // Native GL clear handles it
#endif
}

void text_draw(Value x, Value y, Value size, Value r, Value g, Value b,
               const char *text) {
  (void)size;
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(UTF8ToString($3), $4, $5);
        }
      },
      r, g, b, text, x, y);
#else
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
  glRasterPos2i(x, SCREEN_HEIGHT - y);

  while (*text) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text);
    text++;
  }
  glPopAttrib();
#endif
}

void text_char(Value x, Value y, Value size, Value r, Value g, Value b,
               char c) {
  (void)size;
#ifdef __EMSCRIPTEN__
  EM_ASM_(
      {
        if (window.textCtx) {
          window.textCtx.fillStyle = 'rgb(' + $0 + ',' + $1 + ',' + $2 + ')';
          window.textCtx.fillText(String.fromCharCode($3), $4, $5);
        }
      },
      r, g, b, c, x, y);
#else
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
  glRasterPos2i(x, SCREEN_HEIGHT - y);

  glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
  glPopAttrib();
#endif
}

void text_draw_int(Value x, Value y, Value size, Value r, Value g, Value b,
                   Value value) {
  char buf[32];
  sprintf(buf, "%ld", value);
  text_draw(x, y, size, r, g, b, buf);
}

// ============================================================================
// Math
// ============================================================================

Value math_sin(Value x) { return (Value)(sin(x) * 1000); }

Value math_cos(Value x) { return (Value)(cos(x) * 1000); }

Value math_random(Value min, Value max) {
  return min + rand() % (max - min + 1);
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

void rng_seed(Value seed) { srand((unsigned int)seed); }

Value rng_int(Value max) {
  if (max <= 0)
    return 0;
  return rand() % max;
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
  if (key >= 0 && key < 512)
    keys[key] = 1;
}

void on_key_up(Value key) {
  if (key >= 0 && key < 512)
    keys[key] = 0;
}

void on_frame_start(void) { memcpy(keys_prev, keys, sizeof(keys)); }
