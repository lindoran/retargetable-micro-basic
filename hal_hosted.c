/*
 * hal_hosted.c — Hosted system HAL definitions.
 * This used to live inside BASIC_STAGE1.c; moving it here keeps the
 * interpreter relocatable and lets stubs pull in the right HAL.
 */

#include "hal_base.h"

#if defined(__ia16__) || defined(__MSDOS__) || defined(_MSDOS)
/* -----------------------------------------------------------------------
 * Real DOS: ia16-elf-gcc + libi86, DJGPP, Open Watcom, Turbo C, etc.
 *
 * Port I/O: libi86 and Open Watcom use outp()/inp() from <conio.h>.
 *           Borland Turbo C uses outportb()/inportb() from <dos.h>.
 *           We prefer the Watcom/libi86 names; <dos.h> is NOT included
 *           because its far-pointer typedefs break under -mcmodel=small
 *           with ia16-elf-gcc.
 *
 * delay():  lives in <conio.h> under libi86.  Under DJGPP / Watcom it is
 *           in <dos.h> — if your toolchain puts it there, add <dos.h>
 *           and remove the inline-asm fallback below.
 *
 * Borland target: replace outp/inp with outportb/inportb and add
 *           #include <dos.h> (Borland's dos.h does not use far ptrs).
 * ----------------------------------------------------------------------- */
#  include <conio.h>

/* delay() is in <conio.h> under libi86.  Provide a BIOS-tick fallback
 * for toolchains that lack it (e.g. bare newlib without libi86).        */
#  if !defined(__LIBI86_COMPILING__) && !defined(delay)
static void delay(ubint ms)
{
    /* INT 1Ah AH=00h: read BIOS tick counter (18.2 ticks/sec ~= 1/55ms) */
    unsigned int ticks = ms / 55u + 1u;
    unsigned int start_hi, start_lo, now_hi, now_lo;
    __asm__ volatile (
        "int $0x1a"
        : "=c"(start_hi), "=d"(start_lo)
        : "a"((unsigned int)0x0000)
        : "cc"
    );
    for (;;) {
        __asm__ volatile (
            "int $0x1a"
            : "=c"(now_hi), "=d"(now_lo)
            : "a"((unsigned int)0x0000)
            : "cc"
        );
        /* compare low word only - wraps ~every 24h, fine for short delays */
        if ((unsigned int)(now_lo - start_lo) >= ticks) break;
    }
}
#  endif

void do_beep(ubint freq, ubint ms)
{
    if (freq == 0) { delay(ms); return; }
    ubint divisor = (ubint)(1193180UL / freq);
    outp(0x43, 0xB6);
    outp(0x42, (uint8_t)(divisor & 0xFF));
    outp(0x42, (uint8_t)(divisor >> 8));
    outp(0x61, (uint8_t)(inp(0x61) | 0x03));
    delay(ms);
    outp(0x61, (uint8_t)(inp(0x61) & ~0x03));
}
void  do_delay(ubint ms)          { delay(ms); }
bint  kbtst(void)                 { return (bint)(kbhit() ? getch() : 0); }
ubint do_in(ubint p)              { return (ubint)inp(p); }
void  do_out(ubint p, ubint v)    { outp(p, (uint8_t)v); }

#elif defined(__MINGW32__) || defined(__MINGW64__) || defined(_WIN32)
/* -----------------------------------------------------------------------
 * Windows: MinGW 32/64, MSVC
 * ----------------------------------------------------------------------- */
#  include <windows.h>
#  include <conio.h>

void  do_beep(ubint freq, ubint ms)
{
    if (freq == 0) { Sleep(ms); return; }
    Beep(freq, ms);
}
void  do_delay(ubint ms)            { Sleep(ms); }
bint  kbtst(void)  { return (bint)(_kbhit() ? _getch() : 0); }
ubint do_in(ubint p)               { (void)p; return 0; }
void  do_out(ubint p, ubint v)     { (void)p; (void)v; }

#else
/* -----------------------------------------------------------------------
 * POSIX: Linux / macOS
 * BEEP  -> terminal bell       DELAY -> nanosleep (or clock() fallback)
 * KEY   -> non-blocking termios read
 * INP/OUT -> no-ops (no user-mode port access on protected-mode OS)
 * -----------------------------------------------------------------------
 * Note: stdio.h is included here for getchar() and EOF in kbtst().
 * This is platform-specific HAL code, not interpreter logic.
 * ----------------------------------------------------------------------- */
#  include <stdio.h>
#  include <time.h>
#  ifndef NO_BEEP
#    include <alsa/asoundlib.h>
#    include "tinybeep.h"
void do_beep(ubint freq, ubint ms)
{
    if (freq == 0) { do_delay(ms); return; }
    tinybeep(freq, ms);
}
#  else
void do_beep(ubint freq, ubint ms)
{
    (void)freq;
    do_delay(ms);
}
#  endif

void do_delay(ubint ms)
{
#  if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#  else
    clock_t end = clock() + (clock_t)(ms * (CLOCKS_PER_SEC / 1000));
    while (clock() < end) ;
#  endif
}

#  include <unistd.h>
#  include <termios.h>
#  include <fcntl.h>

bint kbtst(void)
{
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    return (bint)((ch == EOF) ? 0 : ch);
}
ubint do_in(ubint p)           { (void)p; return 0; }
void  do_out(ubint p, ubint v) { (void)p; (void)v; }

#endif /* platform HAL */

void hal_init_audio(void)
{
#ifdef TINYBEEP_H
    snd_lib_error_set_handler(NULL);
#else
    (void)0;
#endif
}
