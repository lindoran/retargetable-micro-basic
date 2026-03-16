/*
 * basic_defs.h — Memory-layout helpers and build-size tuning for BASIC.
 * Consolidates the remaining #if blocks so BASIC_STAGE1.c just includes this
 * header instead of carrying the platform/config conditionals inline.
 */

#ifndef BASIC_DEFS_H
#define BASIC_DEFS_H

#include <stdint.h>

#if defined(AVR_PROGMEM) || defined(__AVR__)
/* Pre-work note: this AVR PROGMEM support is retained for future bare-metal
 * ports. The current repo does not ship an AVR target or io_avr.c. */
#  include <avr/pgmspace.h>
#  define RODATA          PROGMEM
#  define RD_BYTE(p)      pgm_read_byte(p)
#  define RD_PTR(pp)      ((const char *)pgm_read_word(pp))
#else
   /* Von Neumann / hosted: plain dereference, const goes to .rodata      */
#  define RODATA          /* nothing */
#  define RD_BYTE(p)      (*(const uint8_t *)(p))
#  define RD_PTR(pp)      (*(pp))
#endif

#ifdef SMALL_TARGET
#  ifndef BUFFER_SIZE
#    define BUFFER_SIZE   80   /* trim 20 bytes vs default                  */
#  endif
#  ifndef SA_SIZE
#    define SA_SIZE       80   /* must be >= BUFFER_SIZE so literals fit     */
#  endif
#  ifndef NUM_VAR
#    define NUM_VAR      130   /* A0..Z4 : 26*5, halves variable table RAM  */
#  endif
#  ifndef CTL_DEPTH
#    define CTL_DEPTH     24   /* ~4 nested FOR loops or 8 GOSUBs           */
#  endif
#  ifndef MAX_FILES
#    define MAX_FILES      4   /* #0..#3 : enough for CP/M, FLEX, small DOS */
#  endif
#  ifndef SEG_SLOTS
#    define SEG_SLOTS      8   /* [1]..[8] segment cache entries             */
#  endif
#else
#  ifndef BUFFER_SIZE
#    define BUFFER_SIZE  100
#  endif
#  ifndef SA_SIZE
#    define SA_SIZE      100
#  endif
#  ifndef NUM_VAR
#    define NUM_VAR      260   /* A0..Z9 : full variable set                */
#  endif
#  ifndef CTL_DEPTH
#    define CTL_DEPTH     50
#  endif
#  ifndef MAX_FILES
#    define MAX_FILES     10   /* #0..#9                                    */
#  endif
#  ifndef SEG_SLOTS
#    define SEG_SLOTS     16   /* [1]..[16] segment cache entries            */
#  endif
#endif

#if SA_SIZE < BUFFER_SIZE
#  error "SA_SIZE must be >= BUFFER_SIZE (string literals are buffered first)"
#endif

#endif /* BASIC_DEFS_H */
