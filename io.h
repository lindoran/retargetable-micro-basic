/*
 * io.h — Enhanced Micro-BASIC I/O Abstraction Layer
 *
 * This is the sole interface through which BASIC.c performs I/O.
 * All stdio, file operations, and memory allocation are hidden behind
 * these functions. The interpreter is completely independent of the
 * host platform (hosted Linux/Windows/DOS or bare metal AVR/Z80/etc).
 *
 * Implementations:
 *   io_stdio.c   — hosted systems (wraps stdio.h and stdlib.h)
 *   io_avr.c     — bare metal AVR (wraps BIOS layer)
 *   io_stub.c    — minimal no-op template for new ports
 *
 * BASIC.c includes this header only; never includes stdio.h or stdlib.h.
 */

#ifndef IO_H
#define IO_H

#include <stddef.h>   /* size_t */
#include <stdint.h>   /* uint8_t, uint16_t, intptr_t */

/* =======================================================================
 * Console Input/Output
 * ======================================================================= */

/* io_putc(c) — write a single character to console output
 * Never fails silently; buffering and line discipline are io's concern. */
void    io_putc(char c);

/* io_puts(s) — write a NUL-terminated string to console output
 * Equivalent to fputs(s, stdout) but target-agnostic. */
void    io_puts(const char *s);

/* io_flush() — flush pending console output
 * Safe to call even if buffering is not in use (no-op on serial). */
void    io_flush(void);

/* io_getline(buf, maxlen) — read one line from console input
 * Strips trailing CR/LF. Returns number of characters read (not including NUL),
 * or 0 on EOF or error. buf is always NUL-terminated on success. */
int     io_getline(char *buf, int maxlen);

/* =======================================================================
 * File I/O
 * ======================================================================= */

/* IO_FILE — opaque file handle
 * On hosted systems this is a typedef for FILE.
 * On bare metal this is whatever the BIOS provides (FILE, sd_file_t, etc.).
 * BASIC.c never dereferences or inspects this type — only passes it around.
 *
 * Special sentinel values (for hosted systems only):
 * IO_STDIN  — represents console input (from stdin)
 * IO_STDOUT — represents console output (to stdout)
 * These allow BASIC to use console or file I/O interchangeably via the same interface. */
typedef void IO_FILE;

#define IO_STDIN   ((IO_FILE *)(intptr_t)-1)
#define IO_STDOUT  ((IO_FILE *)(intptr_t)-2)

/* io_fopen(name, mode) — open a file
 * mode: "rb" (read binary), "wb" (write binary)
 * Returns NULL on failure (file not found, permission denied, no filesystem).
 * On bare metal without a filesystem, this always returns NULL (LOAD/SAVE
 * become serial operations in io_avr.c, not file operations in the interpreter). */
IO_FILE *io_fopen(const char *name, const char *mode);

/* io_fclose(fp) — close a file
 * Safe to call on NULL (no-op).
 * After close, fp is invalid and should not be used again. */
void     io_fclose(IO_FILE *fp);

/* io_fputc(c, fp) — write one character to a file
 * Safe to call on NULL fp (no-op, no error).
 * buffering and error reporting are io's concern. */
void     io_fputc(char c, IO_FILE *fp);

/* io_fputs(s, fp) — write a NUL-terminated string to a file
 * Safe to call on NULL fp (no-op, no error).
 * Equivalent to fputs(s, fp) but target-agnostic. */
void     io_fputs(const char *s, IO_FILE *fp);

/* io_fgetline(buf, maxlen, fp) — read one line from a file
 * Strips trailing CR/LF. Returns number of characters read (not including NUL),
 * or 0 on EOF or error. buf is always NUL-terminated on success.
 * Safe to call on NULL fp (returns 0 immediately). */
int      io_fgetline(char *buf, int maxlen, IO_FILE *fp);

/* io_flush_file(fp) — flush pending output to a file or console
 * Safe to call on NULL, IO_STDIN, or IO_STDOUT. */
void     io_flush_file(IO_FILE *fp);

/* =======================================================================
 * Memory Management
 * ======================================================================= */

/* io_alloc(size) — allocate memory
 * Equivalent to calloc(1, size) — returns zero-initialized block.
 * On failure, calls io_error() to signal "Out of memory" (error 12)
 * and does not return. The caller can assume io_alloc() always succeeds
 * or terminates the program. */
void    *io_alloc(size_t size);

/* io_free(ptr) — deallocate memory
 * Safe to call on NULL (no-op).
 * On bare metal with a bump allocator, this may be a no-op.
 * On bare metal with an arena allocator, this marks the block freed
 * for reuse. Implementation detail — BASIC.c does not care. */
void     io_free(void *ptr);

/* =======================================================================
 * Error Handling & System Control
 * ======================================================================= */

/* io_error(code) — signal a fatal error and terminate
 * code: BASIC error number (0–13, per error_messages[] in BASIC.c)
 * This function does not return. On hosted systems it calls exit().
 * On bare metal it loops infinitely or triggers a watchdog reset.
 * The implementation is in the target-specific io_xxx.c file. */
void     io_error(int code);

/* io_exit(code) — cleanly terminate the interpreter
 * code: 0 = success, nonzero = error
 * Does not return. On hosted systems calls exit(code).
 * On bare metal loops infinitely or resets via watchdog.
 * The implementation is in the target-specific io_xxx.c file. */
void     io_exit(int code);

int      io_system(const char *cmd);

/* =======================================================================
 * Workspace Interface (for 3.0 bare metal port)
 * ======================================================================= */

/* These are extern'd from the platform-specific io_xxx.c.
 * BASIC.c does not call these — they are for future workspace-based
 * allocation (3.0 embedded port). Documented here for completeness. */

extern uint8_t  *basic_workspace;
extern uint16_t  basic_workspace_size;

#endif /* IO_H */
