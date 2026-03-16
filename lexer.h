/*
 * lexer.h — BASIC tokenizer / lexical analyzer interface
 *
 * The lexer converts raw input lines into token streams.
 * Reserved words become token bytes (index | 0x80).
 * String literals and numeric prefixes are preserved.
 *
 * This is extracted from BASIC.c Stage 1 to enable:
 * - Separate testing of tokenization logic
 * - Fall-through optimization (3-4 char keyword matching)
 * - Future: grammar-driven lexer generation
 */

#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

/* =======================================================================
 * Type aliases (must match BASIC.c exactly)
 * ======================================================================= */

typedef int16_t   bint;     /* BASIC integer: signed 16-bit */
typedef uint16_t  ubint;    /* BASIC unsigned: unsigned 16-bit */
typedef signed char tok_t;  /* Token byte: -128..127 for signed, 0x80..0xFF for tokens */

/* Token macros (match BASIC.c) */
#define TOKEN(k)   ((tok_t)((k) | 0x80))
#define IS_TOK(c)  ((tok_t)(c) < 0)

/* =======================================================================
 * Lexer State
 * ======================================================================= */

/* The lexer works with two pointers (like BASIC.c):
 *   cmdptr — current position in input/token stream
 *   buffer — current line being tokenized
 *
 * These are extern in lexer.c; they're set by the caller (BASIC.c).
 */

extern char *cmdptr;      /* command pointer: current position in line/tokens */
extern char  buffer[100]; /* line buffer: raw input or tokenized output */

/* =======================================================================
 * Token Classification (character type tests)
 * ======================================================================= */

int is_e_end(tok_t c);      /* true at end of expression */
int is_l_end(tok_t c);      /* true at end of line/statement */
int isterm(tok_t c);        /* true if whitespace or line terminator */

/* =======================================================================
 * Lexer Functions
 * ======================================================================= */

/* skip_blank() — advance past whitespace, return current byte
 * Does not consume the returned byte (pure peek + advance). */
tok_t skip_blank(void);

/* get_next() — advance past whitespace, consume and return next byte
 * Returns the byte at current position after skipping whitespace. */
tok_t get_next(void);

/* test_next(token) — check if next non-blank byte equals token
 * If equal, consume it and return 1; otherwise return 0. */
int test_next(tok_t token);

/* expect(token) — require next non-blank byte to equal token
 * If not equal, call error(0) (syntax error). */
void expect(tok_t token);

/* lookup(table) — match next token against reserved word table
 * Returns 1-based index on match (advancing cmdptr), 0 otherwise.
 * table should be a const array of strings, NULL-terminated.
 * Example: lookup(reserved_words) looks for PRINT, IF, FOR, etc. */
ubint lookup(const char * const table[]);

/* get_num() — parse numeric literal from cmdptr
 * Supports:
 *   #xxxx     — hexadecimal (e.g., #FF, #1A2B)
 *   @dddd     — unsigned decimal (e.g., @65535)
 *   dddd      — signed decimal (e.g., 255, -128)
 * Returns ubint; caller casts to bint if needed.
 * Calls error(0) on invalid digits. */
ubint get_num(void);

/* tokenize(line) — tokenize a raw source line in-place
 * Input:  line buffer with raw text (may contain "quoted strings")
 * Output: line buffer with tokens (reserved words → 0x80..0xFF)
 * Returns: 1 if line starts with a line number, 0 otherwise
 *
 * After tokenize(), cmdptr points to start of tokenized line.
 * If line number found, it's already parsed and the function returns 1.
 *
 * This is the main entry point for stage 1 tokenization. */
int tokenize(void);

/* =======================================================================
 * Utility: Reserved Words Table
 * ======================================================================= */

/* The reserved_words table (from BASIC.c) is needed by lookup().
 * It's declared in BASIC.c; the lexer just uses it.
 * Must be declared extern in lexer.c. */
extern const char * const reserved_words[];

#endif /* LEXER_H */
