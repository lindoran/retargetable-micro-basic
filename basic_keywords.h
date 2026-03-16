/*
 * basic_keywords.h — Keyword token definitions for BASIC_STAGE1.c.
 * Keeps the token list separate so the interpreter file stays focused on logic.
 */

#ifndef BASIC_KEYWORDS_H
#define BASIC_KEYWORDS_H

#ifdef OUT
#undef OUT   /* MinGW's windows.h defines OUT as an empty annotation macro */
#endif

/* Primary keyword tokens (1-based; 0 = not found) */
#define LET     1
#define EXIT    2
#define LIST    3
#define NEW     4
#define RUN     5
#define CLEAR   6
#define GOSUB   7
#define GOTO    8
#define RETURN  9
#define PRINT  10
#define FOR    11
#define NEXT   12
#define IF     13
#define LIF    14
#define REM    15
#define STOP   16
#define END    17
#define INPUT  18
#define OPEN   19
#define CLOSE  20
#define DIM    21
#define ORDER  22
#define READ   23
#define DATA   24
#define SAVE   25
#define LOAD   26
#define DELAY  27
#define BEEP   28
#define DOS    29
#define OUT    30
#define SEG    31

/* Secondary keyword tokens */
#define TO     32   /* lower bound of keyword range used in is_e_end() */
#define STEP   33
#define THEN   34

/* Operator / function tokens */
#define ADD    35   /* lower bound of operator range used in is_e_end() */
#define SUB    36
#define MUL    37
#define DIV    38
#define MOD    39
#define AND    40
#define OR     41
#define XOR    42
#define EQ     43
#define NE     44
#define LE     45
#define SHL    46   /* << logical left shift (same priority as & | ^)      */
#define LT     47
#define GE     48
#define SHR    49   /* >> logical right shift (same priority as & | ^)     */
#define GT     50
#define CHR    51
#define STR    52
#define ASC    53
#define ABS    54
#define NUM    55
#define RND    56
#define KEY    57
#define INP    58
#define HEX    59   /* HEX$(n) - format bint as uppercase hex string        */
#define UNS    60   /* UNS$(n) - format bint as unsigned decimal string     */
#define UGT    61   /* UGT(a,b) - unsigned greater-than, returns 1/0        */
#define ULT    62   /* ULT(a,b) - unsigned less-than, returns 1/0           */

/* Pseudo-command: RUN without clearing variables (used by LOAD-in-program) */
#define RUN1   99

#endif /* BASIC_KEYWORDS_H */
