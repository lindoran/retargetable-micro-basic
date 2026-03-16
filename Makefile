# =============================================================================
# ENHANCED MICRO-BASIC 3.0 (Stage 1 Refactored) - Makefile
#
# Architecture:
#   - BASIC_STAGE1.c   : Pure interpreter (no stdio/malloc)
#   - io_<platform>.c  : Platform-specific I/O implementation
#
# Targets:
#   make              -> linux (default)
#   make linux        -> GCC/Linux with ALSA sound
#   make linux-nobeep -> GCC/Linux without sound (fallback)
#   make windows      -> MinGW 32-bit cross (i686-w64-mingw32-gcc)
#   make windows64    -> MinGW 64-bit cross (x86_64-w64-mingw32-gcc)
#   make dos          -> ia16-elf-gcc, PC DOS target (256-640K)
#   (avr target removed for now)
#   make build-all    -> linux, windows, windows64, dos
#   make clean        -> remove all built binaries and packages
#
# Tuning overrides (any target):
#   make linux NUM_VAR=52 CTL_DEPTH=12 SA_SIZE=80
#
# Note: SMALL_TARGET is reserved for the 3.0 embedded port (ATmega/bare metal).
#   It is NOT used for the DOS target -- a standard PC DOS environment has
#   256-640K of conventional memory, which is ample for the normal build.
#   When 3.0 targets bare metal, SMALL_TARGET will require a full I/O HAL
#   replacement (no stdio, no malloc, fixed workspace).  See PORTING.md.
# =============================================================================

# Source files
INTERPRETER = BASIC_STAGE1.c
# Target-specific stubs that pick the proper HAL implementation.
LINUX_STUB     = stubs/linux_stub.c
WINDOWS_STUB   = stubs/windows_stub.c
WINDOWS64_STUB = stubs/windows64_stub.c
DOS_STUB       = stubs/dos_stub.c

# Compiler flags
CFLAGS      = -std=c99 -Wall -Wextra -O2

# Directories
BIN_DIR     = bin
DIST_DIR    = dist
DOCS_DIR    = documents

# Optional tuning defines
ifdef NUM_VAR
  CFLAGS += -DNUM_VAR=$(NUM_VAR)
endif
ifdef CTL_DEPTH
  CFLAGS += -DCTL_DEPTH=$(CTL_DEPTH)
endif
ifdef SA_SIZE
  CFLAGS += -DSA_SIZE=$(SA_SIZE)
endif
ifdef BUFFER_SIZE
  CFLAGS += -DBUFFER_SIZE=$(BUFFER_SIZE)
endif
ifdef MAX_FILES
  CFLAGS += -DMAX_FILES=$(MAX_FILES)
endif

# Interpreter dependencies
INTERPRETER_DEPS = $(INTERPRETER)

# Sound support
HAVE_ALSA ?= 1

# =============================================================================
.PHONY: all linux linux-nobeep windows windows64 dos clean dirs test \
        package-linux package-windows package-windows64 package-dos \
        build-all package-all clean-bins

# Default: linux only
all: linux

dirs:
	mkdir -p $(BIN_DIR) $(DIST_DIR)

# =============================================================================
# Shared documentation staging helper.
# Usage: $(call stage-docs,$(BIN_DIR)/linux)
# Copies documentation files into target dir.
# =============================================================================
define stage-docs
	@if [ -f README.md ]; then cp README.md $(1); fi
	@if [ -f MICRO-BASIC.bnf ]; then cp MICRO-BASIC.bnf $(1); fi
	@if [ -d $(DOCS_DIR) ]; then cp -r $(DOCS_DIR) $(1)/$(DOCS_DIR); fi
	@if [ -f INDEX.md ]; then cp INDEX.md $(1); fi
	@if [ -f PORTING_GUIDE.md ]; then cp PORTING_GUIDE.md $(1); fi
	@if [ -f QUICK_REFERENCE.md ]; then cp QUICK_REFERENCE.md $(1); fi
endef

# =============================================================================
# Linux (with optional ALSA sound)
# =============================================================================
linux: dirs $(INTERPRETER_DEPS) $(LINUX_STUB)
	mkdir -p $(BIN_DIR)/linux
ifeq ($(HAVE_ALSA),1)
	@if [ -f tinybeep.c ]; then \
		echo "Building with ALSA sound..."; \
		gcc $(CFLAGS) -o $(BIN_DIR)/linux/basic $(INTERPRETER_DEPS) $(LINUX_STUB) tinybeep.c -lasound; \
	else \
		echo "Building without sound (tinybeep.c not found)..."; \
		gcc $(CFLAGS) -DNO_BEEP -o $(BIN_DIR)/linux/basic $(INTERPRETER_DEPS) $(LINUX_STUB); \
	fi
else
	@echo "Building without ALSA sound..."
	gcc $(CFLAGS) -DNO_BEEP -o $(BIN_DIR)/linux/basic $(INTERPRETER_DEPS) $(LINUX_STUB)
endif
	@echo "Built: $(BIN_DIR)/linux/basic"
	$(MAKE) package-linux

# Linux without ALSA (fallback)
linux-nobeep: dirs $(INTERPRETER_DEPS) $(LINUX_STUB)
	mkdir -p $(BIN_DIR)/linux
	gcc $(CFLAGS) -DNO_BEEP -o $(BIN_DIR)/linux/basic $(INTERPRETER_DEPS) $(LINUX_STUB)
	@echo "Built: $(BIN_DIR)/linux/basic (no sound)"
	$(MAKE) package-linux

# =============================================================================
# Windows 32-bit (MinGW i686)
# =============================================================================
windows: dirs $(INTERPRETER_DEPS) $(WINDOWS_STUB)
	mkdir -p $(BIN_DIR)/windows
	i686-w64-mingw32-gcc $(CFLAGS) -DNO_BEEP -o $(BIN_DIR)/windows/basic.exe $(INTERPRETER_DEPS) $(WINDOWS_STUB)
	@echo "Built: $(BIN_DIR)/windows/basic.exe"
	$(MAKE) package-windows

# =============================================================================
# Windows 64-bit (MinGW x86_64)
# =============================================================================
windows64: dirs $(INTERPRETER_DEPS) $(WINDOWS64_STUB)
	mkdir -p $(BIN_DIR)/windows64
	x86_64-w64-mingw32-gcc $(CFLAGS) -DNO_BEEP -o $(BIN_DIR)/windows64/basic.exe $(INTERPRETER_DEPS) $(WINDOWS64_STUB)
	@echo "Built: $(BIN_DIR)/windows64/basic.exe"
	$(MAKE) package-windows64

# =============================================================================
# DOS - PC target (ia16-elf-gcc, 256-640K conventional memory)
# =============================================================================
dos: dirs $(INTERPRETER_DEPS) $(DOS_STUB)
	mkdir -p $(BIN_DIR)/dos
	ia16-elf-gcc -mcmodel=small $(CFLAGS) -DNO_BEEP -o $(BIN_DIR)/dos/basic.exe $(INTERPRETER_DEPS) $(DOS_STUB) -li86
	@echo "Built: $(BIN_DIR)/dos/basic.exe"
	$(MAKE) package-dos

# =============================================================================
# Packaging rules
# Each target stages documentation then zips everything.
# =============================================================================

package-linux:
	@echo "Packaging Linux build..."
	$(call stage-docs,$(BIN_DIR)/linux)
	cd $(BIN_DIR)/linux && zip -qr ../../$(DIST_DIR)/linux.zip .

package-windows:
	@echo "Packaging Windows 32-bit build..."
	$(call stage-docs,$(BIN_DIR)/windows)
	cd $(BIN_DIR)/windows && zip -qr ../../$(DIST_DIR)/windows.zip .

package-windows64:
	@echo "Packaging Windows 64-bit build..."
	$(call stage-docs,$(BIN_DIR)/windows64)
	cd $(BIN_DIR)/windows64 && zip -qr ../../$(DIST_DIR)/windows64.zip .

package-dos:
	@echo "Packaging DOS build..."
	$(call stage-docs,$(BIN_DIR)/dos)
	cd $(BIN_DIR)/dos && zip -qr ../../$(DIST_DIR)/dos.zip .

# =============================================================================
# Meta-targets
# =============================================================================

# Build all platforms (hosted systems, not bare metal)
build-all: linux windows windows64 dos

# Package all platforms (assumes builds already done)
package-all: package-linux package-windows package-windows64 package-dos

# =============================================================================
# Clean
# =============================================================================

# Remove everything - binaries and packages
clean:
	rm -rf $(BIN_DIR) $(DIST_DIR)

# Remove only binaries, keep dist packages
clean-bins:
	rm -rf $(BIN_DIR)

# =============================================================================
# Help target
# =============================================================================

help:
	@echo "Enhanced Micro-BASIC 3.0 (Stage 1 Refactored) - Build Targets"
	@echo ""
	@echo "Hosted Systems:"
	@echo "  make linux          - GCC/Linux with ALSA sound"
	@echo "  make linux-nobeep   - GCC/Linux without sound"
	@echo "  make windows        - MinGW 32-bit Windows"
	@echo "  make windows64      - MinGW 64-bit Windows"
	@echo "  make dos            - ia16-elf-gcc DOS (256-640K)"
	@echo "  make build-all      - All of the above"
	@echo ""
	@echo "Utilities:"
	@echo "  make clean          - Remove all binaries and packages"
	@echo "  make clean-bins     - Remove binaries only, keep .zip packages"
	@echo "  make test           - Build linux-nobeep and run BASIC tests"
	@echo "  make help           - This message"
	@echo ""
	@echo "Options:"
	@echo "  NUM_VAR=N           - Number of variables (default 260, small 130, tiny 52)"
	@echo "  CTL_DEPTH=N         - Control stack depth (default 50, small 24)"
	@echo "  SA_SIZE=N           - String accumulator size (default 100, small 80)"
	@echo "  BUFFER_SIZE=N       - Input line buffer size (default 100, small 80)"
	@echo "  MAX_FILES=N         - Number of file handles (default 10, small 4)"
	@echo "  HAVE_ALSA=0         - Disable ALSA sound support (Linux)"
	@echo ""
	@echo "Examples:"
	@echo "  make linux NUM_VAR=52 SA_SIZE=80   - Minimal Linux build"
	@echo "  make dos            - DOS .exe with standard config"
	@echo ""
	@echo "Architecture:"
	@echo "  BASIC_STAGE1.c      - Pure interpreter (no stdio, no malloc)"
	@echo "  stubs/linux_stub.c  - Linux target stub (includes io_stdio.c)"
	@echo "  stubs/windows_stub.c - Windows 32-bit stub (includes io_stdio.c)"
	@echo "  stubs/windows64_stub.c - Windows 64-bit stub (includes io_stdio.c)"
	@echo "  stubs/dos_stub.c    - DOS stub (includes io_stdio.c)"
	@echo "Documentation:"
	@echo "  INDEX.md            - Overview of all deliverables"
	@echo "  PORTING_GUIDE.md    - How to port to new platforms (if present)"
	@echo "  QUICK_REFERENCE.md  - Build commands for common targets (if present)"
	@echo "  STAGE_2_PLAN.md     - Lexer extraction details (if present)"

# =============================================================================
# Tests
# =============================================================================
test: linux-nobeep
	./scripts/run_tests.sh
