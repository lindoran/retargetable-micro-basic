# Porting Guide (Relocation Overview)

This guide summarizes which files matter for a new target and what you typically need to update. It is intended as a quick relocation checklist rather than a deep architecture tutorial.

## Repository Map

Interpreter core (portable):
- `BASIC_STAGE1.c` — the interpreter. No platform headers, no stdio. Should remain untouched for most ports.
- `basic_defs.h` — build-time sizing knobs and PROGMEM helpers.
- `basic_keywords.h` — token table, keyword IDs.
- `basic_types.h` — core typedefs (`bint`, `ubint`, `bptr`).

Hardware abstraction layer (HAL):
- `hal_base.h` — HAL surface the interpreter expects.
- `hal_hosted.c` — hosted implementations (DOS/Windows/POSIX). Use as a reference.

I/O abstraction layer:
- `io.h` — I/O API the interpreter uses.
- `io_stdio.c` — hosted implementation (stdio + malloc). Use as a reference.

Build and stubs:
- `Makefile` — targets and build rules.
- `stubs/*.c` — per-target stubs that include a HAL and I/O implementation.

Documentation:
- `documents/MICRO-BASIC-Manual.md` — language reference.
- `documents/MICRO-BASIC.bnf` — grammar.
- `documents/examples/` — example programs.

## What To Change For A New Port

### 1) Create a Target Stub

Create a new stub in `stubs/<target>_stub.c` that includes your platform HAL and I/O implementation:

```c
#include "../io_<target>.c"
#include "../hal_<target>.c"
```

Hosted targets reuse `io_stdio.c` and `hal_hosted.c`. Bare-metal targets typically provide both files.

### 2) Implement `io_<target>.c`

The interpreter talks to the outside world only via `io.h`. Your `io_<target>.c` must implement:

Console I/O:
- `io_putc`, `io_puts`, `io_flush`, `io_getline`

File I/O (or stubs):
- `io_fopen`, `io_fclose`, `io_fputc`, `io_fputs`, `io_fgetline`, `io_flush_file`

Memory:
- `io_alloc`, `io_free`

Errors and system:
- `io_error`, `io_exit`, `io_system`

If your platform has no filesystem, you can stub file I/O (return NULL / no-op) and route `LOAD`/`SAVE` over serial if desired.

### 3) Implement `hal_<target>.c`

The interpreter calls the HAL from `hal_base.h`:

- `do_beep(freq, ms)`
- `do_delay(ms)`
- `kbtst()`
- `do_in(port)`
- `do_out(port, value)`
- `hal_init_audio()`

On platforms without sound or port I/O, these can be no-ops.

### 4) Update `basic_types.h` If Needed

For non-16-bit native targets or unusual compilers, adjust:

- `bint` (signed 16-bit)
- `ubint` (unsigned 16-bit)
- `bptr` (pointer-width slot)

The interpreter assumes 16-bit integer arithmetic semantics.

### 5) Adjust Build Flags / Sizes

`basic_defs.h` provides size tuning:

- `NUM_VAR`, `CTL_DEPTH`, `SA_SIZE`, `BUFFER_SIZE`, `MAX_FILES`, `SEG_SLOTS`

For small targets, define `SMALL_TARGET` and override values in the Makefile or build command.

### 6) Add a Build Target

Add a `make <target>` rule that compiles:

- `BASIC_STAGE1.c`
- `stubs/<target>_stub.c`

Ensure the proper toolchain flags and any platform libraries are linked.

## Porting Checklist (Quick)

- [ ] Add `stubs/<target>_stub.c`
- [ ] Implement `io_<target>.c`
- [ ] Implement `hal_<target>.c`
- [ ] Add `make <target>` rule
- [ ] Verify `basic_types.h` typedefs
- [ ] Tune `basic_defs.h` sizes
- [ ] Run `make test` (hosted) and a smoke test on target

## Notes

- The interpreter core should not include platform headers.
- All direct system interaction should flow through `io.h` and `hal_base.h`.
- For bare-metal, `io_getline` can be a serial line reader and `io_puts` a serial transmitter.
