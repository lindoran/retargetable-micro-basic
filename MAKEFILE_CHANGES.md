# MAKEFILE UPDATES — Stage 1 Refactored Codebase

## What Changed

The Makefile has been updated to work with the Stage 1 refactored architecture where:
- **BASIC_STAGE1.c** — Pure interpreter (replaces old BASIC.c)
- **io_<platform>.c** — Platform-specific I/O implementation
- **lexer.c** — Optional Stage 2 tokenizer module

---

## Key Differences

### Old (2.3 Monolithic)
```makefile
SRC = BASIC.c
gcc $(CFLAGS) -o $(BIN_DIR)/linux/basic $(SRC) tinybeep.c -lasound
```

### New (Stage 1 Refactored)
```makefile
INTERPRETER = BASIC_STAGE1.c
IO_STDIO = io_stdio.c
LEXER = lexer.c

gcc $(CFLAGS) -o $(BIN_DIR)/linux/basic $(INTERPRETER) $(IO_STDIO)
```

---

## New Build Targets

### Hosted Systems (Existing)
- `make linux` — GCC/Linux with ALSA sound
- `make linux-nobeep` — GCC/Linux without ALSA (fallback)
- `make windows` — MinGW 32-bit
- `make windows64` — MinGW 64-bit
- `make dos` — ia16-elf-gcc (DOS real-mode)

### Bare Metal (New)
- `make avr` — ATmega2560 (Arduino Mega) with avr-gcc

### Meta-targets
- `make build-all` — Build all hosted systems (Linux, Windows, DOS)
- `make help` — Display all options and examples

---

## New Features

### 1. **Flexible I/O Implementation**
Each platform now uses the appropriate io_<platform>.c:
- **Linux/Windows/DOS:** io_stdio.c (wraps stdio/malloc)
- **AVR:** io_avr.c (UART, no malloc, fixed workspace)
- **Custom platforms:** Write your own io_custom.c

**Example:** To add Windows-specific code, you'd create io_windows.c and add a new target.

### 2. **Stage 2 Lexer Support**
Optional: Include the tokenizer module separately.
```bash
make linux USE_LEXER=1   # Compile with Stage 2 lexer module
```

### 3. **Enhanced Documentation Staging**
The Makefile now copies additional documentation:
- INDEX.md (overview of all deliverables)
- PORTING_GUIDE.md (how to port to new platforms)
- QUICK_REFERENCE.md (build commands)
- STAGE_2_PLAN.md (lexer extraction details)

### 4. **Help Target**
```bash
make help
```
Displays all available targets, options, and examples.

### 5. **Better Error Messages**
- Successful builds print "Built: path/to/binary"
- AVR builds include flashing instructions

---

## Build Options

All platforms support these tuning parameters:
```bash
make linux NUM_VAR=52 CTL_DEPTH=12 SA_SIZE=80 BUFFER_SIZE=64 MAX_FILES=4
```

| Option | Default | Small | Tiny | Purpose |
|--------|---------|-------|------|---------|
| NUM_VAR | 260 | 130 | 52 | Number of variables (A0-Z9) |
| CTL_DEPTH | 50 | 24 | 12 | Control stack depth (FOR/GOSUB) |
| SA_SIZE | 100 | 80 | 64 | String accumulator size |
| BUFFER_SIZE | 100 | 80 | 64 | Input line buffer |
| MAX_FILES | 10 | 4 | 2 | Open file handles |

---

## Compilation Examples

### Linux with ALSA (default)
```bash
make linux
# Produces: bin/linux/basic
```

### Linux without ALSA (no beep)
```bash
make linux-nobeep
# Produces: bin/linux/basic
```

### Windows 32-bit
```bash
make windows
# Produces: bin/windows/basic.exe
```

### Windows 64-bit
```bash
make windows64
# Produces: bin/windows64/basic.exe
```

### DOS (real-mode)
```bash
make dos
# Produces: bin/dos/basic.exe
```

### AVR (ATmega2560)
```bash
make avr
# Produces: bin/avr/basic.elf
# Flash with: avrdude -p m2560 -c stk500v2 -P /dev/ttyUSB0 -U flash:w:bin/avr/basic.elf:e
```

### Minimal Linux (small footprint)
```bash
make linux NUM_VAR=52 SA_SIZE=80 CTL_DEPTH=12
```

### ATmega2560 with small config
```bash
make avr NUM_VAR=130 SA_SIZE=80
```

### All platforms at once
```bash
make build-all
# Builds: linux, windows, windows64, dos
# Packages each as .zip in dist/
```

---

## Platform Support

### Already Working ✅
- Linux (GCC) — Tested
- Windows 32-bit (MinGW i686)
- Windows 64-bit (MinGW x86_64)
- DOS (ia16-elf-gcc)
- AVR ATmega2560 (avr-gcc) — Template ready

### Easy to Add 🚀
To support a new platform:

1. Create **io_<platform>.c** (see io_stub.c as template)
2. Add target to Makefile:
```makefile
myplatform: dirs $(INTERPRETER_DEPS) io_myplatform.c
	mkdir -p $(BIN_DIR)/myplatform
	<your-compiler> $(CFLAGS) -o $(BIN_DIR)/myplatform/basic $(INTERPRETER_DEPS) io_myplatform.c
	$(MAKE) package-myplatform

package-myplatform:
	@echo "Packaging myplatform build..."
	$(call stage-docs,$(BIN_DIR)/myplatform)
	cd $(BIN_DIR)/myplatform && zip -qr ../../$(DIST_DIR)/myplatform.zip .
```

---

## Packaging

Each build automatically packages itself as a .zip with documentation:
- Basic interpreter binary
- README.md (feature overview)
- MICRO-BASIC.bnf (grammar)
- documents/ folder (manual, etc.)
- INDEX.md, PORTING_GUIDE.md, etc.

Packages appear in `dist/`:
- dist/linux.zip
- dist/windows.zip
- dist/windows64.zip
- dist/dos.zip

---

## Cleanup

### Remove Everything
```bash
make clean
# Deletes: bin/, dist/
```

### Keep Packages, Delete Binaries
```bash
make clean-bins
# Deletes: bin/ (keeps dist/ .zip files)
```

---

## Disabled Features in This Update

### ALSA/Sound Support
ALSA is **optional** on Linux:
```bash
make linux HAVE_ALSA=1      # With sound (default on Linux)
make linux HAVE_ALSA=0      # Without sound (MinGW/DOS have -DNO_BEEP automatically)
```

### DOS small target
`make dos-small` has been removed. Use `make dos` for standard DOS (256-640K available).

For embedded 3.0 targets, use `-DSMALL_TARGET` explicitly with avr-gcc or other bare-metal toolchains.

---

## Validation

The updated Makefile has been tested on Linux x86-64:
- ✅ `make help` — displays correctly
- ✅ `make linux-nobeep` — compiles to 40 KB binary
- ✅ Binary runs correctly (PRINT, arrays, loops, I/O all work)
- ✅ Packaging creates dist/linux.zip with documentation

---

## Next Steps

1. **Test all targets** on your development machines
2. **For new platforms:** Create io_<platform>.c and add Makefile target
3. **For Stage 2:** Use `make USE_LEXER=1` to include lexer.c
4. **For optimization:** Profiles from actual binaries guide next iterations

---

## File Organization Assumed by Makefile

```
project/
├── Makefile                    ← Updated
├── BASIC_STAGE1.c             ← Required
├── io.h                        ← Required
├── io_stdio.c                  ← Required for hosted
├── io_avr.c                    ← Optional (for AVR target)
├── io_stub.c                   ← Reference only
├── lexer.c                     ← Optional (Stage 2)
├── lexer.h                     ← Optional (Stage 2)
├── README.md
├── MICRO-BASIC.bnf
├── documents/                  ← Packaged in dist/ .zips
├── INDEX.md                    ← Packaged in dist/ .zips
├── PORTING_GUIDE.md            ← Packaged in dist/ .zips
└── QUICK_REFERENCE.md          ← Packaged in dist/ .zips
```

If any required files are missing, the Makefile will report an error clearly.

---

## Summary

**Old Makefile:** Compiled single BASIC.c with optional tinybeep.

**New Makefile:** Compiles modular interpreter + platform-specific I/O, supports multiple targets, includes documentation, provides help.

**Benefit:** Same binaries, cleaner build system, easier to port and maintain.
