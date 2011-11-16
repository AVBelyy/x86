#ifndef pins_macros_for_arduino_compatibility_h
#define pins_macros_for_arduino_compatibility_h

#include <avr/pgmspace.h>
#include <core_pins.h>

#define digitalPinToPort(P) (P)
#define portInputRegister(P) ((volatile uint8_t *)(pgm_read_byte(digital_pin_table_PGM+(P)*2+1)))
#define portModeRegister(P) (portInputRegister(P) + 1)
#define portOutputRegister(P) (portInputRegister(P) + 2)
#define digitalPinToBitMask(P) (pgm_read_byte(digital_pin_table_PGM+(P)*2))
extern const uint8_t PROGMEM digital_pin_table_PGM[];

const static uint8_t SS   = CORE_SS0_PIN;
const static uint8_t MOSI = CORE_MOSI0_PIN;
const static uint8_t MISO = CORE_MISO0_PIN;
const static uint8_t SCK  = CORE_SCLK0_PIN;

#if defined(__AVR_AT90USB162__)
#define digitalPinToPortReg(p) (((p) <= 7) ? &PORTD : (((p) <= 15) ? &PORTB : &PORTC))
#define digitalPinToBit(p) \
	(((p) <= 7) ? (p) : (((p) <= 15) ? (p) - 8 : (((p) <= 19) ? 23 - (p) : 2)))
#define digitalPinToPCICR(p) \
	((((p) >= 8 && (p) <= 15) || ((p) >= 17 && (p) <= 20) || (p) == 5) ? &PCICR : NULL)
#define digitalPinToPCICRbit(p) (((p) >= 8 && (p) <= 15) ? 0 : 1)
#define digitalPinToPCIFR(p) \
	((((p) >= 8 && (p) <= 15) || ((p) >= 17 && (p) <= 20) || (p) == 5) ? &PCIFR : NULL)
#define digitalPinToPCIFRbit(p)	(((p) >= 8 && (p) <= 15) ? 0 : 1)
#define digitalPinToPCMSK(p) \
	(((p) >= 8 && (p) <= 15) ? &PCMSK0 : ((((p) >= 17 && (p) <= 20) || (p) == 5) ? &PCMSK1 : NULL))
#define digitalPinToPCMSKbit(p) \
	(((p) >= 8 && (p) <= 15) ? (p) - 8 : (((p) >= 17 && (p) <= 20) ? (p) - 17 : 4))

#elif defined(__AVR_ATmega32U4__)
#define digitalPinToPortReg(p) \
	(((p) <= 4) ? &PORTB : (((p) <= 8) ? &PORTD : (((p) <= 10) ? &PORTC : (((p) <= 12) ? &PORTD : \
	(((p) <= 15) ? &PORTB : (((p) <= 21) ? &PORTF : (((p) <= 23) ? &PORTD : &PORTE)))))))
#define digitalPinToBit(p) \
	(((p) <= 3) ? (p) : (((p) == 4) ? 7 : (((p) <= 8) ? (p) - 5 : (((p) <= 10) ? (p) - 3 : \
	(((p) <= 12) ? (p) - 5 : (((p) <= 15) ? (p) - 9 : (((p) <= 19) ? 23 - (p) : \
	(((p) <= 21) ? 21 - (p) : (((p) <= 23) ? (p) - 18 : 6)))))))))
#define digitalPinToPCICR(p)	((((p) >= 0 && (p) <= 4) || ((p) >= 13 && (p) <= 15)) ? &PCICR : NULL)
#define digitalPinToPCICRbit(p)	(0)
#define digitalPinToPCIFR(p)	((((p) >= 0 && (p) <= 4) || ((p) >= 13 && (p) <= 15)) ? &PCIFR : NULL)
#define digitalPinToPCIFRbit(p)	(0)
#define digitalPinToPCMSK(p)	((((p) >= 0 && (p) <= 4) || ((p) >= 13 && (p) <= 15)) ? &PCMSK0 : NULL)
#define digitalPinToPCMSKbit(p) \
	(((p) >= 0 && (p) <= 3) ? (p) : (((p) >= 13 && (p) <= 15) ? (p) - 9 : 7))

#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
#define digitalPinToPortReg(p) \
	(((p) >= 0 && (p) <= 7) ? &PORTD : (((p) >= 10 && (p) <= 17) ? &PORTC : \
	(((p) >= 20 && (p) <= 27) ? &PORTB : (((p) >= 28 && (p) <= 35) ? &PORTA : \
	(((p) >= 38 && (p) <= 45) ? &PORTF : &PORTE)))))
#define digitalPinToBit(p) \
	(((p) <= 7) ? (p) : (((p) <= 9) ? (p) - 8 : (((p) <= 17) ? (p) - 10 : \
	(((p) <= 19) ? (p) - 12 : (((p) <= 27) ? (p) - 20 : (((p) <= 35) ? (p) - 28 : \
	(((p) <= 37) ? (p) - 32 : (((p) <= 45) ? (p) - 38 : 2))))))))
#define digitalPinToPCICR(p)	(((p) >= 20 && (p) <= 27) ? &PCICR : NULL)
#define digitalPinToPCICRbit(p)	(0)
#define digitalPinToPCIFR(p)	(((p) >= 20 && (p) <= 27) ? &PCIFR : NULL)
#define digitalPinToPCIFRbit(p)	(0)
#define digitalPinToPCMSK(p)	(((p) >= 20 && (p) <= 27) ? &PCMSK0 : NULL)
#define digitalPinToPCMSKbit(p)	(((p) - 20) & 7)
#endif

#endif
