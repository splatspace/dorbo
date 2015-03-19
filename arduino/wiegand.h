// Handles input for Wiegand readers.
//

#ifndef WIEGAND_H
#define WIEGAND_H

#include <Arduino.h>

#include "config.h"
#include "dorbo_utils.h"

void wiegand_readers_init();

//////////////////////////////////////////////////////////////////////////////
// Wiegand 26-bit Types and Functions
//////////////////////////////////////////////////////////////////////////////

// 24-bit Wiegand (26-bits on-the-wire includes 2 parity bits) as used by 
// HID Prox and many other devices.  Packed for storage in EEPROM.
struct __attribute__((packed)) wiegand26_credential {
  uint8_t facility;
  uint16_t user;
};

#define P_WIEGAND26_CREDENTIAL(e) \
  P("wiegand26_credential<facility="); \
  PDEC((e).facility); \
  P(",user="); \
  PDEC((e).user); \
  P(">");

boolean wiegand_reader_credential_ready(byte reader_num, byte credential_type);
boolean wiegand_reader_get_wiegand26(byte reader_num, struct wiegand26_credential * cred);

// There are Wiegand formats with more than 26 bits.  Adding support for these
// should be straightforward.  Define types here and change the ISR and 
// retrieval code in wiegand.cpp to detect other formats.

#endif
