# nh Language Reference

A NetHack-themed programming language for the langjam.

---

## Quick Reference

| Concept | Syntax |
|---------|--------|
| Statement end | `.` |
| Variable decl | `x := value.` |
| Assignment | `x = value.` |
| Float | `3.14f` |
| Function def | `#name(args) > ... <` |
| Expression fn | `#name(args) => expr.` |
| Function call | `/name/arg1/arg2.` |
| Return | `<< value` |
| Break | `>>` |
| Pipe | `value \| func` |
| Match | `value \| > pattern => result <` |
| Lambda | `\(args) => expr` |
| Logic | `and` `or` `not` |
| Comparison | `lt` `gt` `le` `ge` `==` `!=` |
| Implicit var | `_` |
| Struct | `{ key: value }` |
| Member access | `obj->field` |
| When | `action when cond.` |
| Unless | `action unless cond.` |
| If-else | `a if cond else b` |
| Loop | `loop > ... <` |
| For | `for x in start..end > ... <` |
| Range | `0..10` |

---

## Scope Characters: `>` and `<`

Inspired by NetHack's staircase symbols:
- `>` — stairs down (enter scope)
- `<` — stairs up (exit scope)

```
#main() >
    x := 5.
    << x.
<
```

---

## Literals

```
42                    // Integer
3.14f                 // Float (use f suffix)
"hello"               // String  
true false            // Boolean
[1, 2, 3]             // Array
{ x: 5, y: 10 }       // Struct/Object
```

---

## Variables

```
x := 5.               // Declaration (creates new variable)
MAX := 100.           // Constant (UPPERCASE by convention)
x = 10.               // Assignment (to existing variable)
pi := 3.14f.          // Float variable
name := "hero".       // String variable
arr := [].            // Array variable
```

**Important:** `:=` is for declarations only, `=` is for assignments only.

---

## Operators

### Arithmetic
```
+ - * / %
```

### Comparison (word-based)
```
lt  // less than
gt  // greater than
le  // less or equal
ge  // greater or equal
==  // equal
!=  // not equal
```

### Logical (words, not symbols)
```
and or not
```

---

## Functions

### Block Body
```
#name(args) >
    body.
    << result.        // Return
<
```

### Expression Body
```
#name(args) => expr.
```

### Calling (Wand Syntax)

Functions are called using `/wand/` syntax:

```
/add/5/3.
/print/"hello".
/process/data.
```

**Note:** Use variables for negative numbers or complex expressions:
```
neg := -5.
/func/neg.            // Use variable
/func/(a + b).        // Or parentheses for expressions
```

---

## Structs/Objects

Create structured data with key-value pairs using `{ }`:

```
hero := { hp: 100, x: 5, y: 5 }.
point := { x: 0, y: 0 }.
empty := {}.
```

### Member Access

Use `->` to access fields:

```
x := hero->hp.
pos := hero->x + hero->y.
```

### Member Assignment

```
hero->hp = 80.
hero->x = hero->x + 1.
```

---

## Pipes

Chain operations left-to-right:

```
// Pipe to function
5 | /double/.

// Chain multiple pipes
5 | /double/ | /add_ten/ | /add_ten/.

// Pipe to lambda
5 | \(x) => x * x.
```

When piping to a function, the piped value becomes the first argument.

---

## Pattern Matching

Match a value against patterns:

```
value | >
    0 => "zero"
    1 => "one"
    _ => "other"
<.
```

Use `_` as a wildcard that matches anything:

```
n | >
    0 => 0
    1 => 1
    _ => _ + 10      // _ refers to the matched value
<.
```

---

## Lambdas

Anonymous functions:

```
\(x) => x * 2.
\(a, b) => a + b.
```

Lambdas are most useful when piped:

```
5 | \(x) => x * x.    // Returns 25
```

---

## Control Flow

### Trailing Conditions

Conditions come *after* the action (reads naturally):

```
/attack/enemy when hp gt 50.
/flee/ unless cornered.
/heal/ when hp lt 30.
```

Works with assignments too:
```
x = 10 when flag.
player->hp = 0 when dead.
count = count + 1 when valid.
```

### Inline If-Else

```
value := a if condition else b.
bonus := 2 if critical else 1.
```

### Loops

Basic loop:
```
loop >
    /tick/.
    >> when done.     // Break out
<
```

For-each with range:
```
for i in 0..10 >
    /print/i.
<
```

Range with expressions:
```
for i in start..(end + 1) >
    /process/i.
<
```

Conditional loop:
```
loop when running >
    /update/.
<
```

---

## Arrays

```
arr := [].            // Empty array
arr := [1, 2, 3].     // With elements

arr[0] = 10.          // Assignment
x := arr[0].          // Access
arr[i] = arr[i] + 1.  // Index with expression
```

---

## Comments

```
// Single line comment

/* Multi-line
   comment */
```

---

## Implicit Variable `_`

In pattern matching, `_` refers to the matched value:

```
n | >
    0 => 0
    _ => _ * 2        // _ is n
<.
```

---

## Complete Example

```
// Simple game logic in nh

MAX_HP := 100.

#clamp(val, min_val, max_val) >
    << min_val when val lt min_val.
    << max_val when val gt max_val.
    << val.
<

#create_player() >
    << { hp: 100, x: 5, y: 5 }.
<

#damage(player, amount) >
    new_hp := /clamp/(player->hp - amount)/0/MAX_HP.
    player->hp = new_hp.
    << player->hp.
<

#move(player, dx, dy) >
    player->x = player->x + dx.
    player->y = player->y + dy.
<

#get_status(hp) >
    << hp | >
        0 => 0          // Dead
        _ => 1 if _ lt 30 else 2  // Low or OK
    <.
<

#main() >
    player := /create_player/.
    
    // Damage and check status
    /damage/player/20.
    status := /get_status/player->hp.
    
    // Heal if low
    player->hp = player->hp + 10 when status == 1.
    
    // Movement
    /move/player/1/0.
    
    << 0.
<
```

---

## Design Philosophy

- **`>` `<` for scope** — Like NetHack staircases, descend into code blocks
- **Wand syntax `/fn/args`** — Function calls look like casting spells
- **Statements end with `.`** — Like dungeon crawl text
- **`#name` for functions** — Like NetHack extended commands (#pray, #sit)
- **`->` for member access** — Arrow points to the field
- **Word operators `lt` `gt` `le` `ge`** — Readable comparisons
- **Trailing conditions** — "attack when ready" reads naturally
- **Pipes `|`** — Data flows through transformations
- **Pattern matching** — Match values elegantly
- **No type annotations** — Keep it simple for a jam

## Notes

- **C Reserved Words**: Function names that conflict with C keywords (like `double`, `float`, `int`) are automatically mangled with a `ds_` prefix during compilation.

---

## Runtime Functions

The runtime exposes raw OpenGL bindings plus minimal utilities. This allows you to build graphics abstractions directly in nh!

### Console / Debug

```
/console_log/"message".         // Print string to console
/console_log_int/42.            // Print integer
/console_log_float/3.14f.       // Print float
```

### Input

```
/input_key_pressed/key.         // 1 if key is held, 0 otherwise
/input_key_just_pressed/key.    // 1 if key was just pressed this frame

// Key codes:
// 0=LEFT, 1=RIGHT, 2=UP, 3=DOWN, 4=SPACE, 5=ENTER
// 6=W, 7=A, 8=S, 9=D
```

### Math

```
/math_sin/x.      /math_cos/x.
/math_sqrt/x.     /math_abs/x.
/math_floor/x.    /math_ceil/x.
/math_min/a/b.    /math_max/a/b.
```

### RNG

```
/rng_seed/seed.     // Seed the RNG
/rng_int/max.       // Random int in [0, max)
/rng_float/.        // Random float in [0, 1)
```

### Timing

```
/time_now/.         // Current time in milliseconds
```

### Raw OpenGL Bindings

Shaders:
```
shader := /gl_create_shader/GL_VERTEX_SHADER.
/gl_shader_source_compile/shader/source.
prog := /gl_create_program/.
/gl_attach_shader/prog/shader.
/gl_link_program/prog.
/gl_use_program/prog.
/gl_delete_shader/shader.
```

Uniforms:
```
loc := /gl_get_uniform_location/prog/"name".
/gl_uniform1i/loc/value.
/gl_uniform1f/loc/value.
/gl_uniform2f/loc/x/y.
/gl_uniform3f/loc/x/y/z.
/gl_uniform4f/loc/x/y/z/w.
```

Buffers:
```
buf := /gl_create_buffer/.
/gl_bind_buffer/GL_ARRAY_BUFFER/buf.
/gl_delete_buffer/buf.
```

Vertex Arrays:
```
vao := /gl_create_vertex_array/.
/gl_bind_vertex_array/vao.
/gl_delete_vertex_array/vao.
```

Attributes:
```
loc := /gl_get_attrib_location/prog/"name".
/gl_enable_vertex_attrib_array/loc.
/gl_vertex_attrib_pointer/loc/size/type/normalized/stride/offset.
```

Drawing:
```
/gl_clear_color/r/g/b/a.        // Set clear color (0.0-1.0)
/gl_clear/GL_COLOR_BUFFER_BIT.  // Clear
/gl_viewport/x/y/w/h.           // Set viewport
/gl_enable/GL_BLEND.            // Enable capability
/gl_disable/cap.                // Disable capability
/gl_blend_func/src/dst.         // Set blend function
/gl_draw_arrays/mode/first/count.
/gl_draw_elements/mode/count/type/offset.
```

Float Buffers (for vertex data):
```
handle := /buf_create_floats/count.     // Create CPU buffer
/buf_set_float/handle/index/value.      // Set float at index
/buf_upload/GL_ARRAY_BUFFER/handle/usage. // Upload to GPU
/buf_free/handle.                        // Free buffer
```

### GL Constants

Define these as globals in your nh code:
```
GL_ARRAY_BUFFER := 34962.
GL_STATIC_DRAW := 35044.
GL_DYNAMIC_DRAW := 35048.
GL_VERTEX_SHADER := 35633.
GL_FRAGMENT_SHADER := 35632.
GL_TRIANGLES := 4.
GL_FLOAT := 5126.
GL_BLEND := 3042.
GL_SRC_ALPHA := 770.
GL_ONE_MINUS_SRC_ALPHA := 771.
GL_COLOR_BUFFER_BIT := 16384.
GL_TRUE := 1.
GL_FALSE := 0.
```
