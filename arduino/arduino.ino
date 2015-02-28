
// Arduino IDE requires all library includes to be made from the main sketch file
// (libraries cannot be included from other header files in the sketch).  If this
// bug is ever fixed, you can remove these duplicate includes.
#include <EEPROM.h>
// End duplicate libray includes.
 
#include "config.h"
#include "storage.h"
#include "wiegand.h"
#include "strike.h"
#include "status_led.h"
#include "door_utils.h"
#include "time.h"
#include "cli.h"

void setup() {
  Serial.begin(115200);

  time_init();
  PL("time initialized");
  
  status_led_init();
  PL("status LED initialized");
  
  strike_init();
  PL("strikes initialized");

  wiegand_readers_init();
  PL("wiegand readers initialized");

  cli_init();
  PL("cli initialized");
}

void loop() {
  // Update the shared loop time to avoid having to calculate it in multiple
  // places, or pass 48-bit structs or 16-bit pointers to it.
  time_get(&loop_time, millis());
  
  status_led_loop();
  strike_loop();
  cli_loop();

  // See if any credentials have been presented on the readers.  
  struct wiegand26_credential cred;
  cred.facility = 0;
  cred.user = 0;
  for (byte i = 0; i < NUM_WIEGAND_READERS; i++) {
    if (wiegand_reader_get_wiegand26(i, &cred)) {
      P_WIEGAND26_CREDENTIAL(cred);
      PL();
      if (storage_find_wiegand26_credential(&cred)) {
        PL("credential good");
        strike_open(i);
      } else {
        PL("credential bad");
      }
    }
  }
}


