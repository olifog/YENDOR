#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#endif

// ============================================================================
// Internal State
// ============================================================================

static uint32_t rng_state = 12345;
static int gl_initialized = 0;

#define MAX_KEYS 16
static int key_states[MAX_KEYS] = {0};
static int key_just_pressed[MAX_KEYS] = {0};

// Buffer management for float arrays
#define MAX_FLOAT_BUFFERS 64
#define MAX_BUFFER_SIZE 65536

typedef struct {
    float *data;
    int count;
    int in_use;
} FloatBuffer;

static FloatBuffer float_buffers[MAX_FLOAT_BUFFERS] = {0};

// ============================================================================
// GL Context Initialization
// ============================================================================

static void ensure_gl_context(void) {
#ifdef __EMSCRIPTEN__
    if (gl_initialized) return;
    
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.alpha = 0;
    attrs.depth = 1;
    attrs.stencil = 0;
    attrs.antialias = 1;
    attrs.premultipliedAlpha = 0;
    attrs.preserveDrawingBuffer = 0;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attrs.failIfMajorPerformanceCaveat = 0;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attrs);
    if (ctx <= 0) {
        EM_ASM({
            console.error("Failed to create WebGL2 context:", $0);
        }, ctx);
        return;
    }
    
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(ctx);
    if (res != EMSCRIPTEN_RESULT_SUCCESS) {
        EM_ASM({
            console.error("Failed to make WebGL context current:", $0);
        }, res);
        return;
    }
    
    gl_initialized = 1;
    EM_ASM({
        console.log("[nh] WebGL2 context created successfully");
    });
#endif
}

// ============================================================================
// GL Shader Operations
// ============================================================================

int gl_create_shader(int type) {
#ifdef __EMSCRIPTEN__
    ensure_gl_context();
    return (int)glCreateShader((GLenum)type);
#else
    return 0;
#endif
}

int gl_shader_source_compile(int shader, const char *source) {
#ifdef __EMSCRIPTEN__
    glShaderSource((GLuint)shader, 1, &source, NULL);
    glCompileShader((GLuint)shader);
    
    GLint success;
    glGetShaderiv((GLuint)shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        char log[512];
        glGetShaderInfoLog((GLuint)shader, 512, NULL, log);
        EM_ASM({
            console.error("Shader compile error:", UTF8ToString($0));
        }, log);
        return 0;
    }
    return 1;
#else
    (void)shader; (void)source;
    return 1;
#endif
}

int gl_create_program(void) {
#ifdef __EMSCRIPTEN__
    return (int)glCreateProgram();
#else
    return 0;
#endif
}

void gl_attach_shader(int program, int shader) {
#ifdef __EMSCRIPTEN__
    glAttachShader((GLuint)program, (GLuint)shader);
#else
    (void)program; (void)shader;
#endif
}

int gl_link_program(int program) {
#ifdef __EMSCRIPTEN__
    glLinkProgram((GLuint)program);
    
    GLint success;
    glGetProgramiv((GLuint)program, GL_LINK_STATUS, &success);
    
    if (!success) {
        char log[512];
        glGetProgramInfoLog((GLuint)program, 512, NULL, log);
        EM_ASM({
            console.error("Program link error:", UTF8ToString($0));
        }, log);
        return 0;
    }
    return 1;
#else
    (void)program;
    return 1;
#endif
}

void gl_use_program(int program) {
#ifdef __EMSCRIPTEN__
    glUseProgram((GLuint)program);
#else
    (void)program;
#endif
}

void gl_delete_shader(int shader) {
#ifdef __EMSCRIPTEN__
    glDeleteShader((GLuint)shader);
#else
    (void)shader;
#endif
}

// ============================================================================
// GL Uniforms
// ============================================================================

int gl_get_uniform_location(int program, const char *name) {
#ifdef __EMSCRIPTEN__
    return (int)glGetUniformLocation((GLuint)program, name);
#else
    (void)program; (void)name;
    return -1;
#endif
}

void gl_uniform1i(int location, int v0) {
#ifdef __EMSCRIPTEN__
    glUniform1i((GLint)location, v0);
#else
    (void)location; (void)v0;
#endif
}

void gl_uniform1f(int location, float v0) {
#ifdef __EMSCRIPTEN__
    glUniform1f((GLint)location, v0);
#else
    (void)location; (void)v0;
#endif
}

void gl_uniform2f(int location, float v0, float v1) {
#ifdef __EMSCRIPTEN__
    glUniform2f((GLint)location, v0, v1);
#else
    (void)location; (void)v0; (void)v1;
#endif
}

void gl_uniform3f(int location, float v0, float v1, float v2) {
#ifdef __EMSCRIPTEN__
    glUniform3f((GLint)location, v0, v1, v2);
#else
    (void)location; (void)v0; (void)v1; (void)v2;
#endif
}

void gl_uniform4f(int location, float v0, float v1, float v2, float v3) {
#ifdef __EMSCRIPTEN__
    glUniform4f((GLint)location, v0, v1, v2, v3);
#else
    (void)location; (void)v0; (void)v1; (void)v2; (void)v3;
#endif
}

// ============================================================================
// GL Buffers
// ============================================================================

int gl_create_buffer(void) {
#ifdef __EMSCRIPTEN__
    GLuint buffer;
    glGenBuffers(1, &buffer);
    return (int)buffer;
#else
    return 0;
#endif
}

void gl_bind_buffer(int target, int buffer) {
#ifdef __EMSCRIPTEN__
    glBindBuffer((GLenum)target, (GLuint)buffer);
#else
    (void)target; (void)buffer;
#endif
}

void gl_delete_buffer(int buffer) {
#ifdef __EMSCRIPTEN__
    GLuint b = (GLuint)buffer;
    glDeleteBuffers(1, &b);
#else
    (void)buffer;
#endif
}

// ============================================================================
// GL Vertex Arrays (VAO)
// ============================================================================

int gl_create_vertex_array(void) {
#ifdef __EMSCRIPTEN__
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return (int)vao;
#else
    return 0;
#endif
}

void gl_bind_vertex_array(int vao) {
#ifdef __EMSCRIPTEN__
    glBindVertexArray((GLuint)vao);
#else
    (void)vao;
#endif
}

void gl_delete_vertex_array(int vao) {
#ifdef __EMSCRIPTEN__
    GLuint v = (GLuint)vao;
    glDeleteVertexArrays(1, &v);
#else
    (void)vao;
#endif
}

// ============================================================================
// GL Attributes
// ============================================================================

int gl_get_attrib_location(int program, const char *name) {
#ifdef __EMSCRIPTEN__
    return (int)glGetAttribLocation((GLuint)program, name);
#else
    (void)program; (void)name;
    return -1;
#endif
}

void gl_enable_vertex_attrib_array(int index) {
#ifdef __EMSCRIPTEN__
    glEnableVertexAttribArray((GLuint)index);
#else
    (void)index;
#endif
}

void gl_disable_vertex_attrib_array(int index) {
#ifdef __EMSCRIPTEN__
    glDisableVertexAttribArray((GLuint)index);
#else
    (void)index;
#endif
}

void gl_vertex_attrib_pointer(int index, int size, int type, int normalized, int stride, int offset) {
#ifdef __EMSCRIPTEN__
    glVertexAttribPointer(
        (GLuint)index,
        size,
        (GLenum)type,
        (GLboolean)normalized,
        stride,
        (const void *)(intptr_t)offset
    );
#else
    (void)index; (void)size; (void)type; (void)normalized; (void)stride; (void)offset;
#endif
}

// ============================================================================
// GL Drawing
// ============================================================================

void gl_clear(int mask) {
#ifdef __EMSCRIPTEN__
    glClear((GLbitfield)mask);
#else
    (void)mask;
#endif
}

void gl_clear_color(float r, float g, float b, float a) {
#ifdef __EMSCRIPTEN__
    glClearColor(r, g, b, a);
#else
    (void)r; (void)g; (void)b; (void)a;
#endif
}

void gl_viewport(int x, int y, int width, int height) {
#ifdef __EMSCRIPTEN__
    ensure_gl_context();
    glViewport(x, y, width, height);
#else
    (void)x; (void)y; (void)width; (void)height;
#endif
}

void gl_enable(int cap) {
#ifdef __EMSCRIPTEN__
    glEnable((GLenum)cap);
#else
    (void)cap;
#endif
}

void gl_disable(int cap) {
#ifdef __EMSCRIPTEN__
    glDisable((GLenum)cap);
#else
    (void)cap;
#endif
}

void gl_blend_func(int sfactor, int dfactor) {
#ifdef __EMSCRIPTEN__
    glBlendFunc((GLenum)sfactor, (GLenum)dfactor);
#else
    (void)sfactor; (void)dfactor;
#endif
}

void gl_draw_arrays(int mode, int first, int count) {
#ifdef __EMSCRIPTEN__
    glDrawArrays((GLenum)mode, first, count);
#else
    (void)mode; (void)first; (void)count;
#endif
}

void gl_draw_elements(int mode, int count, int type, int offset) {
#ifdef __EMSCRIPTEN__
    glDrawElements((GLenum)mode, count, (GLenum)type, (const void *)(intptr_t)offset);
#else
    (void)mode; (void)count; (void)type; (void)offset;
#endif
}

// ============================================================================
// Float Buffer Management
// ============================================================================

int buf_create_floats(int count) {
    if (count <= 0 || count > MAX_BUFFER_SIZE) return -1;
    
    for (int i = 0; i < MAX_FLOAT_BUFFERS; i++) {
        if (!float_buffers[i].in_use) {
            float_buffers[i].data = (float *)malloc(count * sizeof(float));
            if (!float_buffers[i].data) return -1;
            memset(float_buffers[i].data, 0, count * sizeof(float));
            float_buffers[i].count = count;
            float_buffers[i].in_use = 1;
            return i;
        }
    }
    return -1;
}

void buf_set_float(int buffer, int index, float value) {
    if (buffer < 0 || buffer >= MAX_FLOAT_BUFFERS) return;
    if (!float_buffers[buffer].in_use) return;
    if (index < 0 || index >= float_buffers[buffer].count) return;
    float_buffers[buffer].data[index] = value;
}

void buf_upload(int target, int buffer_handle, int usage) {
#ifdef __EMSCRIPTEN__
    if (buffer_handle < 0 || buffer_handle >= MAX_FLOAT_BUFFERS) return;
    if (!float_buffers[buffer_handle].in_use) return;
    
    FloatBuffer *buf = &float_buffers[buffer_handle];
    glBufferData((GLenum)target, buf->count * sizeof(float), buf->data, (GLenum)usage);
#else
    (void)target; (void)buffer_handle; (void)usage;
#endif
}

void buf_free(int buffer_handle) {
    if (buffer_handle < 0 || buffer_handle >= MAX_FLOAT_BUFFERS) return;
    if (!float_buffers[buffer_handle].in_use) return;
    
    free(float_buffers[buffer_handle].data);
    float_buffers[buffer_handle].data = NULL;
    float_buffers[buffer_handle].count = 0;
    float_buffers[buffer_handle].in_use = 0;
}

// ============================================================================
// Texture Operations
// ============================================================================

int gl_create_texture(void) {
#ifdef __EMSCRIPTEN__
    GLuint tex;
    glGenTextures(1, &tex);
    return (int)tex;
#else
    return 0;
#endif
}

void gl_bind_texture(int target, int texture) {
#ifdef __EMSCRIPTEN__
    glBindTexture((GLenum)target, (GLuint)texture);
#else
    (void)target; (void)texture;
#endif
}

void gl_tex_parameteri(int target, int pname, int param) {
#ifdef __EMSCRIPTEN__
    glTexParameteri((GLenum)target, (GLenum)pname, param);
#else
    (void)target; (void)pname; (void)param;
#endif
}

// ============================================================================
// Input Implementation
// ============================================================================

int input_key_pressed(int key) {
    if (key < 0 || key >= MAX_KEYS) return 0;
    return key_states[key];
}

int input_key_just_pressed(int key) {
    if (key < 0 || key >= MAX_KEYS) return 0;
    return key_just_pressed[key];
}

EXPORT void on_key_down(int key) {
    if (key >= 0 && key < MAX_KEYS) {
        if (!key_states[key]) {
            key_just_pressed[key] = 1;
        }
        key_states[key] = 1;
    }
}

EXPORT void on_key_up(int key) {
    if (key >= 0 && key < MAX_KEYS) {
        key_states[key] = 0;
    }
}

EXPORT void on_frame_start(void) {
    // Clear just_pressed flags at the start of each frame
    for (int i = 0; i < MAX_KEYS; i++) {
        key_just_pressed[i] = 0;
    }
}

// ============================================================================
// Timing
// ============================================================================

int time_now(void) {
#ifdef __EMSCRIPTEN__
    return (int)emscripten_get_now();
#else
    return 0;
#endif
}

// ============================================================================
// Console
// ============================================================================

void console_log(const char *msg) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log(UTF8ToString($0));
    }, msg);
#else
    printf("%s\n", msg);
#endif
}

void console_log_int(int value) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log($0);
    }, value);
#else
    printf("%d\n", value);
#endif
}

void console_log_float(float value) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log($0);
    }, value);
#else
    printf("%f\n", value);
#endif
}

// ============================================================================
// Math Helpers
// ============================================================================

float math_sin(float x) { return sinf(x); }
float math_cos(float x) { return cosf(x); }
float math_sqrt(float x) { return sqrtf(x); }
float math_floor(float x) { return floorf(x); }
float math_ceil(float x) { return ceilf(x); }
float math_abs(float x) { return fabsf(x); }
float math_min(float a, float b) { return a < b ? a : b; }
float math_max(float a, float b) { return a > b ? a : b; }

// ============================================================================
// RNG
// ============================================================================

void rng_seed(int seed) {
    rng_state = (uint32_t)seed;
}

int rng_int(int max) {
    if (max <= 0) return 0;
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return (int)(rng_state % (uint32_t)max);
}

float rng_float(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return (float)(rng_state & 0x7FFFFF) / (float)0x7FFFFF;
}

// ============================================================================
// Text Rendering (uses 2D canvas overlay via JS)
// ============================================================================

void text_clear(void) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.textCtx) {
            window.textCtx.clearRect(0, 0, 800, 600);
        }
    });
#endif
}

void text_draw(int x, int y, int size, int r, int g, int b, const char *text) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (!window.textCtx) return;
        var ctx = window.textCtx;
        ctx.font = $2 + 'px "Berkeley Mono", monospace';
        ctx.fillStyle = 'rgb(' + $3 + ',' + $4 + ',' + $5 + ')';
        ctx.fillText(UTF8ToString($6), $0, $1 + $2);  // +size for baseline
    }, x, y, size, r, g, b, text);
#else
    (void)x; (void)y; (void)size; (void)r; (void)g; (void)b; (void)text;
#endif
}

void text_char(int x, int y, int size, int r, int g, int b, char c) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (!window.textCtx) return;
        var ctx = window.textCtx;
        ctx.font = $2 + 'px "Berkeley Mono", monospace';
        ctx.fillStyle = 'rgb(' + $3 + ',' + $4 + ',' + $5 + ')';
        ctx.fillText(String.fromCharCode($6), $0, $1 + $2);
    }, x, y, size, r, g, b, (int)c);
#else
    (void)x; (void)y; (void)size; (void)r; (void)g; (void)b; (void)c;
#endif
}

void text_draw_int(int x, int y, int size, int r, int g, int b, int value) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (!window.textCtx) return;
        var ctx = window.textCtx;
        ctx.font = $2 + 'px "Berkeley Mono", monospace';
        ctx.fillStyle = 'rgb(' + $3 + ',' + $4 + ',' + $5 + ')';
        ctx.fillText(String($6), $0, $1 + $2);
    }, x, y, size, r, g, b, value);
#else
    (void)x; (void)y; (void)size; (void)r; (void)g; (void)b; (void)value;
#endif
}
