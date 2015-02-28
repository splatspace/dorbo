#include <Arduino.h>
#include "strike.h"
#include "time.h"

static byte strike_pins[NUM_STRIKES] = STRIKE_PINS;
static unsigned int strike_open_periods[NUM_STRIKES] = STRIKE_OPEN_PERIODS;

struct strike {
  byte pin;
  
  // Close the strike at (and after) this time.  struct time rolls over
  // every ~= 34.8 years.
  struct time close_at;
};

static struct strike strikes[NUM_STRIKES];

void strike_init(void) {
  for (byte i = 0; i < NUM_STRIKES; i++) {
    // Initialize all the fields
    strikes[i].pin = strike_pins[i];
    time_epoch(&strikes[i].close_at);

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
  // Copy the current loop time in
  TIME_CP(strikes[strike_num].close_at, loop_time);
  // Add the configured open time
  time_add(&strikes[strike_num].close_at, strike_open_periods[strike_num]);
}
