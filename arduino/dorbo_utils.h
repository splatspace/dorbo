// Utilities
//

#ifndef DORBO_UTILS_H
#define DORBO_UTILS_H

#include <Arduino.h>

#include "config.h"

// Clear the at the specified SFR address
#define cbi(sfr,bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

// Set the bit at the specified SFR address
#define sbi(sfr,bit) (_SFR_BYTE(sfr) |= _BV(bit))

#ifdef DEBUG
# define IFSER(e)   if (Serial) { e }
# define P(a)       IFSER(Serial.print(a);)
# define PDEC(a)    IFSER(Serial.print(a, DEC);)
# define PBIN(a)    IFSER(Serial.print(a, BIN);)
# define PHEX(a)    IFSER(Serial.print(a, HEX);)
# define PL(a)      IFSER(Serial.println(a);)
# define PLDEC(a)   IFSER(Serial.println(a, DEC);)
# define PLBIN(a)   IFSER(Serial.println(a, BIN);)
# define PLHEX(a)   IFSER(Serial.println(a, HEX);)
#else
# define P(a)
# define PDEC(a)
# define PBIN(a)
# define PHEX(a)
# define PL(a)
# define PLDEC(a)
# define PLBIN(a)
# define PLHEX(a)
#endif

#endif

