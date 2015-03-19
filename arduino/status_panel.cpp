#include <LiquidCrystal.h>

#include "status_panel.h"
#include "dorbo_utils.h"
#include "door.h"

static LiquidCrystal lcd(STATUS_PANEL_LIQUID_CRYSTAL_CONST_ARGS);

#define OPEN_HEART 0
byte open_heart_bytes[8] = {
  0b00000,
  0b01010,
  0b10101,
  0b10001,
  0b10001,
  0b01010,
  0b00100,
  0b00000
};

#define CLOSED_HEART 1
byte closed_heart_bytes[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

void status_panel_init() {
  lcd.createChar(OPEN_HEART, open_heart_bytes);
  lcd.createChar(CLOSED_HEART, closed_heart_bytes);
  lcd.begin(20,4);
  lcd.clear();
  delay(1);
}

void status_panel_loop() {
  for (byte i = 0; i < NUM_DOORS; i++) {
    byte num_printed = 0;
    
    lcd.setCursor(0, i);
    num_printed += lcd.print(i);
    num_printed += lcd.print(':');
    
    long ms_remaining = door_open_remaining_ms(i);
    if (ms_remaining > 0) {
      num_printed += lcd.print("open ");
      num_printed += lcd.print(ms_remaining / 1000);
    } else {
      num_printed += lcd.print("closed");
    }
    
    // Fill the rest of the row with spaces
    for (byte i = num_printed; i < 16; i++) {
      lcd.print(' ');
    }
  }
  
  // The lowest 10 bits of the total millis encode 1024 ms, which is close enough to 
  // 1 second for heartbeat purposes.  Accurate division is very slow on the AVR, 
  // so this is a good trade-off.

  // Light the LED for 256 of the 1024 ms in our period.
  lcd.setCursor(15, 1);
  lcd.write((byte) ((millis() & 0x3ff) < 256 ? CLOSED_HEART : OPEN_HEART));
}

