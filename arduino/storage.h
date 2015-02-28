// EEPROM storage features.
//

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

#include "config.h"
#include "wiegand.h"

// EEPROM data map.  _START are inclusive, _END are exclusive.

// 24-bit Wiegand codes (26-bit on the wire with 2 parity bits, but these are not stored)
#define WIEGAND26_ZONE_START       0
#define WIEGAND26_ZONE_CRED_SIZE   3
#define WIEGAND26_ZONE_END         (WIEGAND26_ZONE_START + (WIEGAND26_ZONE_CRED_SIZE * WIEGAND26_MAX_CREDS))
#define WIEGAND26_ZONE_ADDR(I)     (WIEGAND26_ZONE_START + (WIEGAND26_ZONE_CRED_SIZE * (I)))

// Add other storage zones here

// Check that the last byte of the last zone will fit.
#ifndef E2END
# error E2END not defined for this architecture.
#elif (WIEGAND_ZONE_END - 1) > E2END
# error Not enough room on this chip for the configured EEPROM data.  Consider adjusting the storage limits.
#endif

// Wiegand functions

boolean storage_write_wiegand26_credential(int index, struct wiegand26_credential * c);
boolean storage_read_wiegand26_credential(int index, struct wiegand26_credential * c);
boolean storage_find_wiegand26_credential(struct wiegand26_credential * c);

#endif

