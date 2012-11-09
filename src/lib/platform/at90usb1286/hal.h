#ifndef WProgram_h
#define WProgram_h

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef GCC_VERSION
#if (GCC_VERSION < 40300)
#warning "Your avr-gcc and avr-libc are too old, please upgrade"
#endif
#if (GCC_VERSION >= 40300) && (GCC_VERSION < 40302)
// gcc 4.3.0 fails to save context for some interrupt routines - very ugly
#warning "Buggy GCC 4.3.0 compiler, please upgrade!"
#endif
#endif

#include <avr/interrupt.h>
#include <wiring.h>

#ifdef __cplusplus

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
const static uint8_t A0 = CORE_ANALOG0_PIN;
const static uint8_t A1 = CORE_ANALOG1_PIN;
const static uint8_t A2 = CORE_ANALOG2_PIN;
const static uint8_t A3 = CORE_ANALOG3_PIN;
const static uint8_t A4 = CORE_ANALOG4_PIN;
const static uint8_t A5 = CORE_ANALOG5_PIN;
const static uint8_t A6 = CORE_ANALOG6_PIN;
const static uint8_t A7 = CORE_ANALOG7_PIN;
#if defined(__AVR_ATmega32U4__)
const static uint8_t A8 = CORE_ANALOG8_PIN;
const static uint8_t A9 = CORE_ANALOG9_PIN;
const static uint8_t A10 = 10;
const static uint8_t A11 = CORE_ANALOG11_PIN;
#endif
#endif

#endif // __cplusplus

#endif // WProgram_h
