#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

// ============================================================================
// Platform Detection
// ============================================================================

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES3/gl3.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#define EMSCRIPTEN_KEEPALIVE
// Stub GL types for native compilation (testing only)
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned int GLenum;
typedef int GLsizei;
typedef float GLfloat;
typedef unsigned char GLboolean;
typedef char GLchar;
#endif

// ============================================================================
// Screen Constants
// ============================================================================

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// ============================================================================
// Raw GL Constants (subset of WebGL2 / OpenGL ES 3.0)
// These are exposed so .nh code can use them directly
// ============================================================================

// Buffer targets
#define GL_ARRAY_BUFFER_C         0x8892
#define GL_ELEMENT_ARRAY_BUFFER_C 0x8893

// Buffer usage
#define GL_STATIC_DRAW_C          0x88E4
#define GL_DYNAMIC_DRAW_C         0x88E8
#define GL_STREAM_DRAW_C          0x88E0

// Shader types
#define GL_VERTEX_SHADER_C        0x8B31
#define GL_FRAGMENT_SHADER_C      0x8B30

// Primitives
#define GL_POINTS_C               0x0000
#define GL_LINES_C                0x0001
#define GL_LINE_STRIP_C           0x0003
#define GL_TRIANGLES_C            0x0004
#define GL_TRIANGLE_STRIP_C       0x0005
#define GL_TRIANGLE_FAN_C         0x0006

// Data types
#define GL_FLOAT_C                0x1406
#define GL_UNSIGNED_BYTE_C        0x1401
#define GL_UNSIGNED_SHORT_C       0x1403
#define GL_UNSIGNED_INT_C         0x1405

// Enable caps
#define GL_BLEND_C                0x0BE2
#define GL_DEPTH_TEST_C           0x0B71
#define GL_CULL_FACE_C            0x0B44

// Blend functions
#define GL_SRC_ALPHA_C            0x0302
#define GL_ONE_MINUS_SRC_ALPHA_C  0x0303
#define GL_ONE_C                  0x0001
#define GL_ZERO_C                 0x0000

// Clear bits
#define GL_COLOR_BUFFER_BIT_C     0x4000
#define GL_DEPTH_BUFFER_BIT_C     0x0100

// Texture
#define GL_TEXTURE_2D_C           0x0DE1
#define GL_TEXTURE0_C             0x84C0
#define GL_TEXTURE_MIN_FILTER_C   0x2801
#define GL_TEXTURE_MAG_FILTER_C   0x2800
#define GL_TEXTURE_WRAP_S_C       0x2802
#define GL_TEXTURE_WRAP_T_C       0x2803
#define GL_NEAREST_C              0x2600
#define GL_LINEAR_C               0x2601
#define GL_CLAMP_TO_EDGE_C        0x812F
#define GL_REPEAT_C               0x2901
#define GL_RGBA_C                 0x1908

// Boolean
#define GL_TRUE_C                 1
#define GL_FALSE_C                0

// ============================================================================
// Raw GL Bindings - Shader Operations
// ============================================================================

// Create a shader (type: GL_VERTEX_SHADER_C or GL_FRAGMENT_SHADER_C)
int gl_create_shader(int type);

// Set shader source and compile (returns shader ID, 0 on failure)
int gl_shader_source_compile(int shader, const char *source);

// Create a program
int gl_create_program(void);

// Attach shader to program
void gl_attach_shader(int program, int shader);

// Link program (returns 1 on success, 0 on failure)
int gl_link_program(int program);

// Use program
void gl_use_program(int program);

// Delete shader
void gl_delete_shader(int shader);

// ============================================================================
// Raw GL Bindings - Uniforms
// ============================================================================

// Get uniform location
int gl_get_uniform_location(int program, const char *name);

// Set uniforms
void gl_uniform1i(int location, int v0);
void gl_uniform1f(int location, float v0);
void gl_uniform2f(int location, float v0, float v1);
void gl_uniform3f(int location, float v0, float v1, float v2);
void gl_uniform4f(int location, float v0, float v1, float v2, float v3);

// ============================================================================
// Raw GL Bindings - Buffers
// ============================================================================

// Create a buffer
int gl_create_buffer(void);

// Bind buffer (target: GL_ARRAY_BUFFER_C or GL_ELEMENT_ARRAY_BUFFER_C)
void gl_bind_buffer(int target, int buffer);

// Delete buffer
void gl_delete_buffer(int buffer);

// ============================================================================
// Raw GL Bindings - Vertex Arrays (VAO)
// ============================================================================

// Create a vertex array object
int gl_create_vertex_array(void);

// Bind VAO
void gl_bind_vertex_array(int vao);

// Delete VAO
void gl_delete_vertex_array(int vao);

// ============================================================================
// Raw GL Bindings - Attributes
// ============================================================================

// Get attribute location
int gl_get_attrib_location(int program, const char *name);

// Enable vertex attribute array
void gl_enable_vertex_attrib_array(int index);

// Disable vertex attribute array
void gl_disable_vertex_attrib_array(int index);

// Set vertex attribute pointer
// (index, size, type, normalized, stride, offset)
void gl_vertex_attrib_pointer(int index, int size, int type, int normalized, int stride, int offset);

// ============================================================================
// Raw GL Bindings - Drawing
// ============================================================================

// Clear the screen
void gl_clear(int mask);

// Set clear color (0.0 - 1.0)
void gl_clear_color(float r, float g, float b, float a);

// Set viewport
void gl_viewport(int x, int y, int width, int height);

// Enable/disable capability
void gl_enable(int cap);
void gl_disable(int cap);

// Blend function
void gl_blend_func(int sfactor, int dfactor);

// Draw arrays
void gl_draw_arrays(int mode, int first, int count);

// Draw elements
void gl_draw_elements(int mode, int count, int type, int offset);

// ============================================================================
// Buffer Data - Float arrays
// We need special handling since .nh doesn't have real arrays
// ============================================================================

// Create a float buffer in C memory, returns handle
int buf_create_floats(int count);

// Set float at index in buffer
void buf_set_float(int buffer, int index, float value);

// Upload buffer to GL (target, buffer_handle, usage)
void buf_upload(int target, int buffer_handle, int usage);

// Free buffer
void buf_free(int buffer_handle);

// ============================================================================
// Texture Operations (simplified)
// ============================================================================

// Create texture
int gl_create_texture(void);

// Bind texture
void gl_bind_texture(int target, int texture);

// Set texture parameter
void gl_tex_parameteri(int target, int pname, int param);

// ============================================================================
// Input API (minimal)
// ============================================================================

#define KEY_LEFT  0
#define KEY_RIGHT 1
#define KEY_UP    2
#define KEY_DOWN  3
#define KEY_SPACE 4
#define KEY_ENTER 5
#define KEY_W     6
#define KEY_A     7
#define KEY_S     8
#define KEY_D     9

// Check if a key is currently pressed (1 = pressed, 0 = not)
int input_key_pressed(int key);

// Check if a key was just pressed this frame
int input_key_just_pressed(int key);

// ============================================================================
// Timing
// ============================================================================

// Get current time in milliseconds
int time_now(void);

// ============================================================================
// Console / Debug
// ============================================================================

void console_log(const char *msg);
void console_log_int(int value);
void console_log_float(float value);

// ============================================================================
// Text Rendering (uses 2D canvas overlay)
// ============================================================================

// Clear the text overlay
void text_clear(void);

// Draw text at position with size and color (RGB 0-255)
void text_draw(int x, int y, int size, int r, int g, int b, const char *text);

// Draw a single character (for dungeon rendering)
void text_char(int x, int y, int size, int r, int g, int b, char c);

// Draw an integer at position
void text_draw_int(int x, int y, int size, int r, int g, int b, int value);

// ============================================================================
// Math Helpers (since .nh has limited math)
// ============================================================================

float math_sin(float x);
float math_cos(float x);
float math_sqrt(float x);
float math_floor(float x);
float math_ceil(float x);
float math_abs(float x);
float math_min(float a, float b);
float math_max(float a, float b);

// ============================================================================
// RNG
// ============================================================================

void rng_seed(int seed);
int rng_int(int max);
float rng_float(void);

// ============================================================================
// Game Loop Exports (called from JS)
// ============================================================================

EXPORT void game_init(void);
EXPORT void game_update(int dt);
EXPORT void game_render(void);

// Input callbacks (called from JS)
EXPORT void on_key_down(int key);
EXPORT void on_key_up(int key);
EXPORT void on_frame_start(void);

#endif // RUNTIME_H
