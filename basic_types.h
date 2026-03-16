/*
 * basic_types.h — Portable BASIC scalar types and token helpers.
 * Shared between BASIC_STAGE1.c and HAL implementations so both see
 * the same definitions without duplicating typedefs.
 */

#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <stdint.h>

typedef int16_t   bint;     /* BASIC numeric type   - signed 16-bit         */
typedef uint16_t  ubint;    /* BASIC unsigned type  - unsigned 16-bit        */
typedef uintptr_t bptr;     /* pointer-width slot   - ctl_stk entries        */

typedef signed char tok_t;
#define TOKEN(k)   ((tok_t)((k) | 0x80))
#define IS_TOK(c)  ((tok_t)(c) < 0)

#endif /* BASIC_TYPES_H */
