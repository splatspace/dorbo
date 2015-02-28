// Utilities
//

#ifndef DOOR_UTILS_H
#define DOOR_UTILS_H

#include <Arduino.h>

#include "config.h"

// Clear the at the specified SFR address
#define cbi(sfr,bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

// Set the bit at the specified SFR address
#define sbi(sfr,bit) (_SFR_BYTE(sfr) |= _BV(bit))

#ifdef DEBUG
# define P(a)       Serial.print(a)
# define PDEC(a)    Serial.print(a, DEC)
# define PBIN(a)    Serial.print(a, BIN)
# define PHEX(a)    Serial.print(a, HEX)
# define PL(a)      Serial.println(a)
# define PLDEC(a)   Serial.println(a, DEC)
# define PLBIN(a)   Serial.println(a, BIN)
# define PLHEX(a)   Serial.println(a, HEX)
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

