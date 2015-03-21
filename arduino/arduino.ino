// Arduino IDE requires all library includes to be made from the main sketch file
// (libraries cannot be included from other header files in the sketch).  If this
// bug is ever fixed, you can remove these duplicate includes.
#include <EEPROM.h>
#include <LiquidCrystal.h>
// End duplicate libray includes.
 
#include "config.h"
#include "storage.h"
#include "wiegand.h"
#include "door.h"
#include "status_panel.h"
#include "dorbo_utils.h"
#include "cli.h"

// For testing millis() rollover.
extern volatile unsigned long timer0_millis;

void setup() {
  Serial.begin(115200);

  status_panel_init();
  PL("status panel initialized");
  
  door_init();
  PL("doors initialized");

  wiegand_readers_init();
  PL("wiegand readers initialized");

  cli_init();
  PL("cli initialized");
  
  // For testing millis() rollover.
  //timer0_millis = -5000;
}

void loop() {
  status_panel_loop();
  door_loop();
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
        door_open(i);
      } else {
        PL("credential bad");
      }
    }
  }
}


