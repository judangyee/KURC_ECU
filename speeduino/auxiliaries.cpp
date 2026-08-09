/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "auxiliaries.h"

/* VVT (variable valve timing) and WMI (water-methanol injection) control were
 * removed from this fork (single-cylinder build, no VVT/WMI hardware).
 *
 * Both functions are kept as no-ops rather than deleted outright:
 * - initialiseAuxPWM() is called unconditionally from init.cpp.
 * - vvtInterrupt() is called unconditionally from the AVR TIMER1_COMPB_vect
 *   ISR (board_avr2560.cpp). Since nothing enables that timer any more
 *   (ENABLE_VVT_TIMER() was only ever called from the removed vvtControl()),
 *   the ISR never actually fires, but the symbol must still exist to link.
 */
void initialiseAuxPWM(void) { }
void vvtInterrupt(void) { }
