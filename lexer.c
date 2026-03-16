/*
 * lexer.c — BASIC tokenizer / lexical analyzer
 *
 * Extracted from BASIC.c Stage 1 to enable:
 * - Separate module and testing
 * - Future fall-through optimization (3-4 char keyword matching)
 * - Platform to optimize tokenization independently
 *
 * This module is pure state-machine lexing with no I/O or interpreter logic.
 * It reads from cmdptr (command pointer) and writes to buffer (line buffer).
 */

#include "lexer.h"
#include "io.h"
#include <ctype.h>
#include <string.h>

/* =======================================================================
 * External State (owned by BASIC.c)
 * ======================================================================= */

/* These are declared and initialized in BASIC.c */
extern char *cmdptr;      /* command pointer: position in current line */
extern char  buffer[100]; /* line buffer: input or tokenized output */

/* Reserved words table (in BASIC.c) — used by lookup() */
extern const char * const reserved_words[];

/* =======================================================================
 * Character Classification Helpers
 * ======================================================================= */

int is_e_end(tok_t c)
{
    if (c >= TOKEN(TO) && c < TOKEN(ADD)) return 1;
    return (c == '\0') || (c == ':') || (c == ')') || (c == ']') || (c == ',') || (c == ';');
}

int is_l_end(tok_t c)
{
    return (c == '\0') || (c == ':');
}

int isterm(tok_t c)
{
    return (c == ' ') || (c == '\t');
}

/* =======================================================================
 * Skip and Get Primitives
 * ======================================================================= */

tok_t skip_blank(void)
{
    while (isterm((tok_t)*cmdptr)) ++cmdptr;
    return (tok_t)*cmdptr;
}

tok_t get_next(void)
{
    tok_t c;
    while (isterm((tok_t)(c = (tok_t)*cmdptr))) ++cmdptr;
    if (c) ++cmdptr;
    return c;
}

int test_next(tok_t token)
{
    if (skip_blank() == token) { ++cmdptr; return 1; }
    return 0;
}

void expect(tok_t token)
{
    if (get_next() != token) {
        /* Would call error(0) — but lexer module should not know about
         * error handling. Caller will check return values instead.
         * For now, we'll call a stub that BASIC.c will override. */
        extern void error(unsigned int);
        error(0);
    }
}

/* =======================================================================
 * Keyword Lookup
 * ======================================================================= */

ubint lookup(const char * const table[])
{
    ubint       i;
    const char *cptr;
    char       *optr = cmdptr;

    for (i = 0; (cptr = table[i]) != NULL; ++i) {
        while (*cptr && (*cptr == toupper((unsigned char)*cmdptr))) {
            ++cptr; ++cmdptr;
        }
        if (!*cptr) {
            /* Avoid matching "FOR" inside "FORMAT": reject if both the
             * last-matched char and the very next input char are alnum. */
            if (!(isalnum((unsigned char)*(cptr-1)) &&
                  isalnum((unsigned char)*cmdptr))) {
                skip_blank();
                return (ubint)(i + 1);
            }
        }
        cmdptr = optr;
    }
    return 0;
}

/* =======================================================================
 * Numeric Literal Parser
 * ======================================================================= */

ubint get_num(void)
{
    ubint value = 0;
    char  c;

    c = *cmdptr;

    if (c == '#') {                         /* --- hexadecimal --- */
        ++cmdptr;
        if (!isxdigit((unsigned char)*cmdptr)) {
            extern void error(unsigned int);
            error(0);
        }
        while (isxdigit((unsigned char)(c = *cmdptr))) {
            ubint digit;
            ++cmdptr;
            if      (c >= '0' && c <= '9') digit = (ubint)(c - '0');
            else if (c >= 'a' && c <= 'f') digit = (ubint)(c - 'a' + 10);
            else                           digit = (ubint)(c - 'A' + 10);
            if (value > (ubint)0x0FFF) {
                extern void error(unsigned int);
                error(0);   /* would overflow ubint */
            }
            value = (ubint)((value << 4) | digit);
        }

    } else if (c == '@') {                  /* --- unsigned decimal --- */
        ubint tmp;
        ++cmdptr;
        if (!isdigit((unsigned char)*cmdptr)) {
            extern void error(unsigned int);
            error(0);
        }
        while (isdigit((unsigned char)(c = *cmdptr))) {
            ++cmdptr;
            tmp = (ubint)(value * 10 + (ubint)(c - '0'));
            if (tmp < value) {
                extern void error(unsigned int);
                error(0);   /* wrapped: value > 65535 */
            }
            value = tmp;
        }

    } else {                                /* --- plain signed decimal --- */
        while (isdigit((unsigned char)(c = *cmdptr))) {
            ++cmdptr;
            value = (ubint)(value * 10 + (ubint)(c - '0'));
        }
    }

    return value;
}

/* =======================================================================
 * Main Tokenization Entry Point
 * ======================================================================= */

int tokenize(void)
{
    ubint  value;
    char  *ptr;
    tok_t  c;

    /* Strip trailing CR/LF from input line */
    {
        char *nl = buffer + strlen(buffer);
        while (nl > buffer && (*(nl-1) == '\n' || *(nl-1) == '\r')) {
            *--nl = '\0';
        }
    }

    /* Tokenise: replace reserved words with (index | 0x80) bytes
     * String literals are preserved as-is */
    cmdptr = ptr = buffer;
    while ((c = (tok_t)*cmdptr) != 0) {
        if ((value = lookup(reserved_words)) != 0) {
            *ptr++ = (char)(value | 0x80);
        } else {
            *ptr++ = (char)c;
            ++cmdptr;
            if (c == '"') {
                /* Pass string literals verbatim */
                while ((c = (tok_t)*cmdptr) && c != '"') {
                    ++cmdptr;
                    *ptr++ = (char)c;
                }
                *ptr++ = *cmdptr++;
            }
        }
    }
    *ptr   = '\0';
    cmdptr = buffer;

    /* Check if line starts with a line number (numbered line) */
    if (isdigit((unsigned char)skip_blank())) {
        value = get_num();
        /* Return 1 to indicate this is a numbered line.
         * The line number is in 'value' and cmdptr has advanced past it.
         * Caller (BASIC.c) handles delete/insert. */
        return 1;
    }

    /* Unnumbered line (direct command) */
    return 0;
}

/* =======================================================================
 * Token Type Constants (from BASIC.c lexer)
 * ======================================================================= */

/* These macros and constants are defined in BASIC.c.
 * They're referenced here for clarity but aren't duplicated.
 * Keyword token indices (1-based; used by lookup):
 *   LET=1, EXIT=2, LIST=3, ..., ULT=62
 * These are combined with 0x80 (high bit set) to create token bytes.
 */

#define TO     32
#define ADD    35
