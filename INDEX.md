# Build Index

Use this as the one–page map for building and retargeting Enhanced Micro-Basic. All commands are run from the repo root.
See `documents/PORTING_GUIDE.md` for a file-by-file relocation checklist.

## Hosted Targets (current)

| Target      | Command            | Stub / HAL               | Output                 | Notes                          |
|-------------|--------------------|--------------------------|------------------------|--------------------------------|
| Linux       | `make linux`       | `stubs/linux_stub.c` → `hal_hosted.c`, `io_stdio.c` | `bin/linux/basic`     | ALSA sound if `HAVE_ALSA=1` and `tinybeep.c` present |
| Windows 32  | `make windows`     | `stubs/windows_stub.c` → `hal_hosted.c`, `io_stdio.c` | `bin/windows/basic.exe` | MinGW i686 cross-compiler required |
| Windows 64  | `make windows64`   | `stubs/windows64_stub.c` → `hal_hosted.c`, `io_stdio.c` | `bin/windows64/basic.exe` | MinGW x86_64 cross-compiler required |
| DOS (ia16)  | `make dos`         | `stubs/dos_stub.c` → `hal_hosted.c`, `io_stdio.c` | `bin/dos/basic.exe`    | Uses `ia16-elf-gcc`, no ALSA sound |

`make build-all` builds all hosted targets; each `make <target>` also creates a matching `.zip` under `dist/`.

## Stubs and HAL Selection

- Hosted stubs live in `stubs/*_stub.c` and simply include `io_stdio.c` and `hal_hosted.c`.
- `hal_base.h` defines the HAL surface (`do_beep`, `do_delay`, `kbtst`, `do_in`, `do_out`, `hal_init_audio`).
- To add a new target, create `stubs/<target>_stub.c` that pulls in your platform HAL instead of `hal_hosted.c`, then add a `<target>` rule in the Makefile pointing `INTERPRETER_DEPS` to that stub.

## Tuning Knobs (all targets)

Pass any of these to `make <target>`:

- `NUM_VAR`, `CTL_DEPTH`, `SA_SIZE`, `BUFFER_SIZE`, `MAX_FILES` – control variable tables, stack depth, buffers.
- `HAVE_ALSA=0` – force no sound on Linux.

Defaults are defined in `basic_defs.h`.

## Relocation Guide (quick start)

1) Implement a HAL for your platform that satisfies `hal_base.h` and `io.h` expectations.  
2) Create a stub `stubs/<target>_stub.c` that includes your HAL and the appropriate `io_*` backend.  
3) Add a Makefile rule `<target>`: `INTERPRETER_DEPS = $(INTERPRETER) stubs/<target>_stub.c` and set the right compiler/toolchain.  
4) If no filesystem exists, implement `io_system`, `io_fopen/io_fclose/io_fgetline` as no-ops or serial equivalents.  
5) Verify with `make <target>` and bundle docs via the existing packaging step.
