#include <Arduino.h>
#include "strike.h"
#include "time.h"

struct strike {
  byte pin;
  
  // Close the strike at (and after) this time.  Struct time rolls over every
  // 139 years.
  struct time close_at;
};

struct strike strikes[NUM_STRIKES];

void strike_init(void) {
  for (byte i = 0; i < NUM_STRIKES; i++) {
    // Initialize all the fields
    strikes[i].pin = strike_pins[i];
    strikes[i].close_at.seconds = 0;
    strikes[i].close_at.millis = 0;

    // Configure as output pins
    pinMode(strikes[i].pin, OUTPUT);
    digitalWrite(strikes[i].pin, LOW);
  }
}

void strike_loop() {
  for (byte i = 0; i < NUM_STRIKES; i++) {
    boolean open = TIME_LT_TIME(loop_time, strikes[i].close_at);
    digitalWrite(strikes[i].pin, open ? HIGH : LOW);
  }
}

void strike_open(byte strike_num) {
  // Copy the current time in
  TIME_CP(strikes[strike_num].close_at, loop_time);
  // Add the configured amount
  time_add(&strikes[strike_num].close_at, strike_open_periods[strike_num], 0);
}
