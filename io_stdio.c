/*
 * io_stdio.c — I/O implementation for hosted systems (Linux / Windows / DOS)
 *
 * Wraps stdio.h and stdlib.h. This is the reference implementation —
 * when porting to bare metal (3.0), replace with io_avr.c, io_z80.c, etc.
 *
 * This file is the ONLY place in the codebase that includes:
 *   stdio.h, stdlib.h, string.h
 * BASIC.c and all other interpreter code use only io.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"

/* =======================================================================
 * Workspace interface (unused on hosted systems, required for 3.0 porting)
 * ======================================================================= */

/* On hosted systems we use malloc, so workspace is not used.
 * For 3.0 bare metal porting, the io_avr.c implementation will set these
 * to point to the actual RAM. BASIC.c never touches these. */

uint8_t  *basic_workspace      = NULL;
uint16_t  basic_workspace_size = 0;

/* =======================================================================
 * Console Output
 * ======================================================================= */

void io_putc(char c)
{
    putc(c, stdout);
}

void io_puts(const char *s)
{
    fputs(s, stdout);
}

void io_flush(void)
{
    fflush(stdout);
}

/* =======================================================================
 * Console Input
 * ======================================================================= */

int io_getline(char *buf, int maxlen)
{
    char *r;
    int len;

    if (!buf || maxlen < 1) return 0;

    r = fgets(buf, maxlen, stdin);
    if (!r) return 0;

    /* fgets keeps the newline; strip it along with any trailing CR */
    len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[--len] = '\0';
    }

    return len;
}

/* =======================================================================
 * File I/O
 * ======================================================================= */

/* IO_FILE is just FILE on hosted systems */
IO_FILE *io_fopen(const char *name, const char *mode)
{
    return (IO_FILE *)fopen(name, mode);
}

void io_fclose(IO_FILE *fp)
{
    /* Don't close stdin/stdout sentinels */
    if (fp == IO_STDIN || fp == IO_STDOUT) return;
    if (fp) fclose((FILE *)fp);
}

/* io_flush_file(fp) — flush pending output to file or console.
 * Handles sentinel values and NULL safely. */
void io_flush_file(IO_FILE *fp)
{
    if (fp == IO_STDOUT) { fflush(stdout); }
    else if (fp == IO_STDIN) { /* no-op for input */ }
    else if (fp) { fflush((FILE *)fp); }
}

void io_fputc(char c, IO_FILE *fp)
{
    if (fp == IO_STDOUT) { putc(c, stdout); }
    else if (fp == IO_STDIN) { /* writing to stdin is a no-op */ }
    else if (fp) { putc(c, (FILE *)fp); }
}

void io_fputs(const char *s, IO_FILE *fp)
{
    if (!s) return;
    if (fp == IO_STDOUT) { fputs(s, stdout); }
    else if (fp == IO_STDIN) { /* writing to stdin is a no-op */ }
    else if (fp) { fputs(s, (FILE *)fp); }
}

int io_fgetline(char *buf, int maxlen, IO_FILE *fp)
{
    char *r;
    int len;

    if (!fp || !buf || maxlen < 1) return 0;

    /* IO_STDOUT is write-only, return 0 for read attempts */
    if (fp == IO_STDOUT) return 0;

    /* IO_STDIN is console input */
    if (fp == IO_STDIN) {
        r = fgets(buf, maxlen, stdin);
    } else {
        r = fgets(buf, maxlen, (FILE *)fp);
    }

    if (!r) return 0;

    /* fgets keeps the newline; strip it along with any trailing CR */
    len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[--len] = '\0';
    }

    return len;
}

/* =======================================================================
 * Memory Management
 * ======================================================================= */

void *io_alloc(size_t size)
{
    void *ptr = calloc(1, size);
    if (!ptr) {
        /* Error 12 = "Out of memory"
         * We could call error(12) but that's in BASIC.c and we want to
         * keep dependencies one-way (BASIC.c → io.h, never the reverse).
         * Instead we call io_error() which the interpreter expects. */
        io_error(12);
    }
    return ptr;
}

void io_free(void *ptr)
{
    if (ptr) free(ptr);
}

/* =======================================================================
 * Error Handling & System Control
 * ======================================================================= */

void io_error(int code)
{
    (void)code;
    /* On hosted systems, io_error() is only called from io_alloc() failure.
     * The interpreter's error() function is in BASIC.c and does the normal
     * error reporting. This is the fatal "out of memory" path. */
    fputs("Fatal: Out of memory\n", stderr);
    exit(12);
}

void io_exit(int code)
{
    exit(code);
}

int io_system(const char *cmd)
{
#if defined(__ia16__) || defined(__MSDOS__) || defined(_MSDOS)
#  if defined(_DOS_SYSTEM_DEFINED)
    if (!cmd) return -1;
    return _dos_system(cmd);
#  else
    (void)cmd;
    return -1;
#  endif
#else
    return system(cmd);
#endif
}

/* ======================================================================= */
