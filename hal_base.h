/*
 * hal_base.h — HAL interface prototypes that the interpreter expects.
 * Each stub provides these symbols by pulling in a platform-specific HAL.
 */

#ifndef HAL_BASE_H
#define HAL_BASE_H

#include "basic_types.h"

void do_beep(ubint freq, ubint ms);
void do_delay(ubint ms);
bint kbtst(void);
ubint do_in(ubint port);
void do_out(ubint port, ubint value);

void hal_init_audio(void);

#endif /* HAL_BASE_H */
