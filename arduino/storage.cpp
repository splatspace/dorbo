#include <EEPROM.h>
#include "storage.h"

// Wiegand functions

boolean storage_write_wiegand26_credential(int index, struct wiegand26_credential * cred) {
  if (index >= WIEGAND26_MAX_CREDS) {
    return false;
  }
  EEPROM.write(WIEGAND26_ZONE_ADDR(index), cred->facility);
  EEPROM.write(WIEGAND26_ZONE_ADDR(index) + 1, (cred->user >> 8) & 0xff);
  EEPROM.write(WIEGAND26_ZONE_ADDR(index) + 2, (cred->user) & 0xff);
  return true;
}

boolean storage_read_wiegand26_credential(int index, struct wiegand26_credential * c) {
  if (index >= WIEGAND26_MAX_CREDS) {
    return false;
  }
  c->facility = EEPROM.read(WIEGAND26_ZONE_ADDR(index));
  c->user = (EEPROM.read(WIEGAND26_ZONE_ADDR(index) + 1) << 8) 
          | (EEPROM.read(WIEGAND26_ZONE_ADDR(index) + 2) << 0);
  return true;
}

boolean storage_find_wiegand26_credential(struct wiegand26_credential * c) {
  struct wiegand26_credential candidate;
  for (int i = 0; i < WIEGAND26_MAX_CREDS; i++) {
    storage_read_wiegand26_credential(i, &candidate);
    if (c->facility == candidate.facility && c->user == candidate.user) {
      return true;
    }
  }
  return false;
}
  

