#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

// Basic types
// Basic types
typedef long Value;

// Tagging: Integers are odd (bit 0 = 1), Pointers are passed through unchanged
// 0 is represented as 1 (VAL_INT(0))
#define VAL_INT(x) ((long)(((unsigned long)(x) << 1) | 1))
#define VAL_OBJ(x) ((Value)(x))
#define AS_INT(x) ((long)(x) >> 1)
#define AS_OBJ(x)                                                              \
  ((void *)(x)) // Don't modify pointer - WASM strings may be at odd addresses
#define IS_INT(x) (((x) & 1))
#define IS_OBJ(x) (!((x) & 1))

// ============================================================================
// Garbage Collection - Root Registration
// ============================================================================

// Register an array as a GC root (call at init for all global arrays that may
// hold strings)
void gc_register_root_array(Value *array, int size);

// Register a single value as a GC root (call at init for all global variables
// that may hold strings)
void gc_register_root_value(Value *value_ptr);

// Execution stack protection - protect temporary environments during execution
void gc_push_env(Value env);
void gc_pop_env(void);

// Force a garbage collection cycle
void gc_force_collect(void);

// Clear the execution stack (call when interpreter stops)
void gc_clear_exec_stack(void);

// Clear a root array (set all elements to 0 so GC can collect old objects)
void gc_clear_array(Value *array, Value size_val);

// Check if value is a string (vs integer)
Value ds_is_string(Value v);

// Get string length
Value ds_string_length(Value str_val);

// Convert integer to string
Value ds_int_to_string(Value int_val);

// Convert object to string (JSON-like)
Value ds_object_to_string(Value obj_val);

// Convert any value to string (JSON-like, auto-detect list/obj)
Value ds_val_to_string(Value val);

// ============================================================================
// Platform Detection
// ============================================================================

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten.h>
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
// Screen Constants (reference resolution)
// ============================================================================

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Get actual screen dimensions (may differ from constants on web)
Value get_screen_width(void);
Value get_screen_height(void);

// Set whether to skip CRT effects (for intro/menus)
void set_skip_crt(Value skip);

// ============================================================================
// Raw GL Constants (subset of WebGL2 / OpenGL ES 3.0)
// These are exposed so .nh code can use them directly
// ============================================================================

// Buffer targets
#define GL_ARRAY_BUFFER_C 0x8892
#define GL_ELEMENT_ARRAY_BUFFER_C 0x8893

// Buffer usage
#define GL_STATIC_DRAW_C 0x88E4
#define GL_DYNAMIC_DRAW_C 0x88E8
#define GL_STREAM_DRAW_C 0x88E0

// Shader types
#define GL_VERTEX_SHADER_C 0x8B31
#define GL_FRAGMENT_SHADER_C 0x8B30

// Primitives
#define GL_POINTS_C 0x0000
#define GL_LINES_C 0x0001
#define GL_LINE_STRIP_C 0x0003
#define GL_TRIANGLES_C 0x0004
#define GL_TRIANGLE_STRIP_C 0x0005
#define GL_TRIANGLE_FAN_C 0x0006

// Data types
#define GL_FLOAT_C 0x1406
#define GL_UNSIGNED_BYTE_C 0x1401
#define GL_UNSIGNED_SHORT_C 0x1403
#define GL_UNSIGNED_INT_C 0x1405

// Enable caps
#define GL_BLEND_C 0x0BE2
#define GL_DEPTH_TEST_C 0x0B71
#define GL_CULL_FACE_C 0x0B44

// Blend functions
#define GL_SRC_ALPHA_C 0x0302
#define GL_ONE_MINUS_SRC_ALPHA_C 0x0303
#define GL_ONE_C 0x0001
#define GL_ZERO_C 0x0000

// Clear bits
#define GL_COLOR_BUFFER_BIT_C 0x4000
#define GL_DEPTH_BUFFER_BIT_C 0x0100

// Texture
#define GL_TEXTURE_2D_C 0x0DE1
#define GL_TEXTURE0_C 0x84C0
#define GL_TEXTURE_MIN_FILTER_C 0x2801
#define GL_TEXTURE_MAG_FILTER_C 0x2800
#define GL_TEXTURE_WRAP_S_C 0x2802
#define GL_TEXTURE_WRAP_T_C 0x2803
#define GL_NEAREST_C 0x2600
#define GL_LINEAR_C 0x2601
#define GL_CLAMP_TO_EDGE_C 0x812F
#define GL_REPEAT_C 0x2901
#define GL_RGBA_C 0x1908

// Boolean
#define GL_TRUE_C 1
#define GL_FALSE_C 0

// ============================================================================
// Raw GL Bindings - Shader Operations
// ============================================================================

// Create a shader (type: GL_VERTEX_SHADER_C or GL_FRAGMENT_SHADER_C)
Value gl_create_shader(Value type);

// Set shader source and compile (returns shader ID, 0 on failure)
Value gl_shader_source_compile(Value shader, Value source);

// Create a program
Value gl_create_program(void);

// Attach shader to program
void gl_attach_shader(Value program, Value shader);

// Link program (returns 1 on success, 0 on failure)
Value gl_link_program(Value program);

// Use program
void gl_use_program(Value program);

// Delete shader
void gl_delete_shader(Value shader);

// ============================================================================
// Raw GL Bindings - Uniforms
// ============================================================================

// Get uniform location
Value gl_get_uniform_location(Value program, Value name);

// Set uniforms
void gl_uniform1i(Value location, Value v0);
void gl_uniform1f(Value location, float v0);
void gl_uniform2f(Value location, float v0, float v1);
void gl_uniform3f(Value location, float v0, float v1, float v2);
void gl_uniform4f(Value location, float v0, float v1, float v2, float v3);

// ============================================================================
// Raw GL Bindings - Buffers
// ============================================================================

// Create a buffer
Value gl_create_buffer(void);

// Bind buffer (target: GL_ARRAY_BUFFER_C or GL_ELEMENT_ARRAY_BUFFER_C)
void gl_bind_buffer(Value target, Value buffer);

// Delete buffer
void gl_delete_buffer(Value buffer);

// ============================================================================
// Raw GL Bindings - Vertex Arrays (VAO)
// ============================================================================

// Create a vertex array object
Value gl_create_vertex_array(void);

// Bind VAO
void gl_bind_vertex_array(Value vao);

// Delete VAO
void gl_delete_vertex_array(Value vao);

// ============================================================================
// Raw GL Bindings - Attributes
// ============================================================================

// Get attribute location
Value gl_get_attrib_location(Value program, Value name);

// Enable vertex attribute array
void gl_enable_vertex_attrib_array(Value index);

// Disable vertex attribute array
void gl_disable_vertex_attrib_array(Value index);

// Set vertex attribute pointer
// (index, size, type, normalized, stride, offset)
void gl_vertex_attrib_pointer(Value index, Value size, Value type,
                              Value normalized, Value stride, Value offset);

// ============================================================================
// Raw GL Bindings - Drawing
// ============================================================================

// Clear the screen
void gl_clear(Value mask);

// Set clear color (0.0 - 1.0)
void gl_clear_color(float r, float g, float b, float a);

// Set viewport
void gl_viewport(Value x, Value y, Value width, Value height);

// Enable/disable capability
void gl_enable(Value cap);
void gl_disable(Value cap);

// Blend function
void gl_blend_func(Value sfactor, Value dfactor);

// Scissor test for clipping
void gl_scissor(Value x, Value y, Value width, Value height);
void set_clip_rect(Value x, Value y, Value w, Value h);
void clear_clip_rect(void);

// Draw arrays
void gl_draw_arrays(Value mode, Value first, Value count);

// Draw elements
void gl_draw_elements(Value mode, Value count, Value type, Value offset);

// ============================================================================
// Buffer Data - Float arrays
// We need special handling since .nh doesn't have real arrays
// ============================================================================

// Create a float buffer in C memory, returns handle
Value buf_create_floats(Value count);

// Set float at index in buffer
void buf_set_float(Value buffer, Value index, float value);

// Upload buffer to GL (target, buffer_handle, usage)
void buf_upload(Value target, Value buffer_handle, Value usage);

// Free buffer
void buf_free(Value buffer_handle);

// ============================================================================
// Texture Operations (simplified)
// ============================================================================

// Create texture
Value gl_create_texture(void);

// Bind texture
void gl_bind_texture(Value target, Value texture);

// Set texture parameter
void gl_tex_parameteri(Value target, Value pname, Value param);

// ============================================================================
// Input API (minimal)
// ============================================================================

// Key constants - use RT_ prefix to avoid conflicts with nh code
#define RT_KEY_LEFT 0
#define RT_KEY_RIGHT 1
#define RT_KEY_UP 2
#define RT_KEY_DOWN 3
#define RT_KEY_SPACE 4
#define RT_KEY_ENTER 5
#define RT_KEY_W 6
#define RT_KEY_A 7
#define RT_KEY_S 8
#define RT_KEY_D 9
#define RT_KEY_TAB 10
#define RT_KEY_BACKSPACE 11
#define RT_KEY_DELETE 12
#define RT_KEY_HOME 13
#define RT_KEY_END 14
#define RT_KEY_ESCAPE 15

// Letter keys: 100 + (0-25) for a-z
// Digit keys: 200 + (0-9) for 0-9
// Special keys: 210+ for symbols

// Check if a key is currently pressed (1 = pressed, 0 = not)
Value input_key_pressed(Value key);

// Check if a key was just pressed this frame
Value input_key_just_pressed(Value key);

// Check if shift key is held
Value input_shift_held(void);

// Shift key callbacks
EXPORT void on_shift_down(void);
EXPORT void on_shift_up(void);

// Character input (keyboard-layout-independent)
EXPORT void on_char_input(Value char_code); // Called from JS
Value input_get_char(void); // Get and consume next character (0 if none)
Value input_has_char(void); // Check if characters are available

// ============================================================================
// Timing
// ============================================================================

// Get current time in milliseconds
Value time_now(void);

// ============================================================================
// Console / Debug
// ============================================================================

void console_log(Value msg);
void console_log_int(Value value);
void console_log_float(float value);

// ============================================================================
// Text Rendering (uses 2D canvas overlay)
// ============================================================================

// Clear the text overlay
void text_clear(void);

// Draw text at position with size and color (RGB 0-255)
void text_draw(Value x, Value y, Value size, Value r, Value g, Value b,
               Value text);

// Draw a single character (for dungeon rendering)
void text_char(Value x, Value y, Value size, Value r, Value g, Value b,
               Value c);

// Draw an integer at position
void text_draw_int(Value x, Value y, Value size, Value r, Value g, Value b,
                   Value value);

// Draw text with a custom font (for intro/special screens)
// font_id: 0 = serif, 1 = handwriting, 2 = typewriter
void text_draw_font(Value x, Value y, Value size, Value r, Value g, Value b,
                    Value font_id, Value text_val);

// Draw text right-aligned to x position (text ends at x)
void text_draw_font_right(Value x, Value y, Value size, Value r, Value g,
                          Value b, Value font_id, Value text_val);

// Measure text width in pixels (uses monospace font)
Value text_measure(Value text_val, Value font_size);

// Draw a filled rectangle
void draw_rect(Value x, Value y, Value w, Value h, Value r, Value g, Value b,
               Value a);

// Draw a line segment (for signature drawing)
void draw_line(Value x1, Value y1, Value x2, Value y2, Value width, Value r,
               Value g, Value b, Value a);

// ============================================================================
// Math Helpers (since .nh has limited math)
// ============================================================================

Value math_sin(Value x);
Value math_cos(Value x);
float math_sqrt(float x);
float math_floor(float x);
float math_ceil(float x);
float math_abs(float x);
float math_min(float a, float b);
float math_max(float a, float b);

// ============================================================================
// RNG
// ============================================================================

void rng_seed(Value seed);
Value rng_int(Value max);
float rng_float(void);

// ============================================================================
// Game Loop Exports (called from JS)
// ============================================================================

EXPORT void game_init(void);
EXPORT void game_update(Value dt);
EXPORT void game_render(void);

// Input callbacks (called from JS)
EXPORT void on_key_down(Value key);
EXPORT void on_key_up(Value key);
EXPORT void on_frame_start(void);

// ============================================================================
// Object System (for .nh structs)
// ============================================================================

// Simple dynamic object system
// Basic types
typedef long Value;

// IO
void text_write(const char *str);
void text_write_int(Value val);
Value time_ms(void);

// GL
void gl_color(Value r, Value g, Value b, Value a);
void gl_begin(Value mode);
void gl_end(void);
void gl_vertex(Value x, Value y);
void gl_flush(void);

// Input
Value input_key_pressed(Value key);

// Math
Value math_sin(Value x);
Value math_cos(Value x);
Value math_random(Value min, Value max);

// Object System
Value ds_object_create(Value count_val, ...);
Value ds_object_get(Value obj, Value key);
void ds_object_set(Value *obj, Value key, Value value);
void ds_set_prop(Value obj, Value key, Value value);

// String helpers
Value ds_strlen(Value str);
Value ds_string_at(Value str, Value index);
Value ds_substring(Value str, Value start, Value len);
Value ds_streq(Value s1, Value s2);
Value ds_div(Value a, Value b);
Value ds_mod(Value a, Value b);
Value ds_is_string_like(Value val);
Value val_eq(Value a, Value b);

// String manipulation for text editing
Value ds_string_insert_char(Value str, Value pos, Value char_code);
Value ds_string_delete_char(Value str, Value pos);
Value ds_string_concat(Value str1, Value str2);
Value ds_char_to_string(Value char_code);

// Mouse input
Value input_mouse_x(void);
Value input_mouse_y(void);
Value input_mouse_down(void);
Value input_mouse_just_pressed(void);
Value input_mouse_just_released(void);

// Scroll wheel input
Value input_scroll_delta(void);
void input_scroll_clear(void);

// Clipboard
Value clipboard_copy_requested(void);
Value clipboard_paste_requested(void);
Value clipboard_get_text(void);
void clipboard_set_text(Value str);
void clipboard_clear_requests(void);
Value select_all_requested(void);

// List helpers
Value ds_list_create(void);
Value ds_list_push(Value list, Value value);
Value ds_list_get(Value list, Value index);
Value ds_list_len(Value list);
Value ds_is_list(Value val);
Value ds_is_object(Value val);
Value ds_list_to_string(Value list);
Value ds_json_encode(Value val);

// ============================================================================
// Audio
// ============================================================================
void play_sound(Value id);

// ============================================================================
// Volume Control
// ============================================================================
void js_call_set_volume(Value level);
void js_call_set_master_volume(Value level);
void js_call_set_music_volume(Value level);
void js_call_set_sfx_volume(Value level);
void js_call_muffle_music(void);
void js_call_unmuffle_music(void);

// ============================================================================
// URL Opening (opens in new tab)
// ============================================================================
void open_url(Value url);

// ============================================================================
// Save/Load System
// ============================================================================
void js_call_save_game(Value slot, Value code, Value upgrades,
                       Value chat_state);
Value js_call_load_game(Value slot);
Value js_call_save_exists(Value slot);

// ============================================================================
// Settings Persistence
// ============================================================================
Value js_call_get_setting(Value key);
void js_call_set_setting(Value key, Value value);

// ============================================================================
// JSON Parsing Helpers
// ============================================================================
Value js_parse_save_code(Value json);
Value js_parse_save_upgrades(Value json);
Value js_parse_save_chat_state(Value json);

// Menu Art Fetching
Value js_get_menu_art_count(void);
Value js_get_menu_art_line(Value idx);

#endif // RUNTIME_H
