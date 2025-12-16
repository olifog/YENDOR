# nh 🎮

A langjam project: a NetHack-inspired programming language for building roguelike bots.

## The Language

**nh** uses NetHack-themed syntax:

```
#main() >
    player := { hp: 100, x: 0, y: 0 }.
    
    loop >
        /move/player/1/0.
        >> when player->x ge 10.
    <
    
    << 0.
<
```

### Key Features

- **`>` `<` for scope** — Like NetHack staircases
- **`/wand/` syntax** — Function calls are spells
- **`#command`** — Functions named like extended commands
- **Word operators** — `lt` `gt` `le` `ge` `and` `or` `not`
- **Pipes & pattern matching** — Functional programming flow
- **Trailing conditions** — `attack when ready.`

See [LANGUAGE.md](LANGUAGE.md) for full documentation.

## Building

### Prerequisites

```bash
# macOS
brew install bison flex

# Ubuntu/Debian
sudo apt install bison flex
```

### Build & Test

```bash
make              # Build compiler
make test         # Run test suite
./build/dsc examples/hello.nh # Compile a file
```

### Test the QAD interpreter

```bash
make test-interpreter
```

If you want an example of how to use it, look in the `run_test` function in `interpreter/main.nh`.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                       nh Compiler                            │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌──────────┐ │
│  │  Flex   │───▶│  Bison  │───▶│   AST   │───▶│ C Codegen│ │
│  │ (lexer) │    │(parser) │    │         │    │          │ │
│  └─────────┘    └─────────┘    └─────────┘    └──────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                         Generated C
                              │
                              ▼
                     gcc / emcc → binary / WASM
```

## Project Structure

```
langjam/
├── compiler/           # nh compiler (Bison/Flex → C)
│   ├── lexer.l         # Flex lexer
│   ├── parser.y        # Bison parser
│   ├── ast.h/c         # AST node definitions
│   └── codegen.c       # C code generator
├── runtime/            # Runtime library
│   └── runtime.h/c     # Graphics, input, logging
├── tests/              # Test suite (40 tests)
│   ├── *.nh            # Test files
│   └── run_tests.sh    # Test runner
├── examples/           # Example programs
│   └── hello.nh
├── LANGUAGE.md         # Language reference
└── Makefile
```

## Quick Syntax Reference

| Concept | Syntax |
|---------|--------|
| Function | `#name(args) > body <` |
| Call | `/func/arg1/arg2.` |
| Return | `<< value.` |
| Break | `>>.` |
| Variable | `x := 5.` |
| Struct | `{ key: value }` |
| Member | `obj->field` |
| Loop | `loop > body <` |
| For | `for i in 0..10 > body <` |
| Pattern | `val \| > 0 => a _ => b <` |
| Pipe | `5 \| /double/ \| /inc/` |
| Lambda | `\(x) => x * 2` |
| When | `action when cond.` |
| Comparison | `lt` `gt` `le` `ge` `==` `!=` |
| Logic | `and` `or` `not` |

---

*Built for Langjam 2025* 🎮
