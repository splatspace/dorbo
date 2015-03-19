#include <Arduino.h>
#include "door.h"

// Declared and defined by config.h
static byte strike_pins[NUM_DOORS] = DOOR_STRIKE_PINS;
static byte accepted_led_pins[NUM_DOORS] = DOOR_ACCEPTED_LED_PINS;
static byte denied_led_pins[NUM_DOORS] = DOOR_DENIED_LED_PINS;
static unsigned int strike_open_periods[NUM_DOORS] = DOOR_STRIKE_OPEN_PERIODS;

// A half-open interval used for holding strikes open.
struct interval {
  // The millis value that defines the (open; inclusive) start of the interval
  uint32_t start;
  // The duration in ms that is added to the start to calculate the 
  // (closed; exclusive) end of the interval.
  uint32_t duration;
};

// Roll-over safe
#define INTERVAL_CONTAINS(i,ms) ((ms) - (i).start <= (i).duration)

static struct interval hold_open[NUM_DOORS];

void door_init(void) {
  for (byte i = 0; i < NUM_DOORS; i++) {
    // Initialize the intervals so the doors are initially closed
    hold_open[i].start = 0;
    hold_open[i].duration = 0;

    // LOW is strike closed (door locked), HIGH is strike open (door unlocked)
    pinMode(strike_pins[i], OUTPUT);
    digitalWrite(strike_pins[i], LOW);
    
    // LOW is LED off, HIGH is LED on
    if (accepted_led_pins[i] != 255) {
      pinMode(accepted_led_pins[i], OUTPUT);
      digitalWrite(accepted_led_pins[i], LOW);
    }
    
    // LOW is LED off, HIGH is LED on
    if (denied_led_pins[i] != 255) {
      pinMode(denied_led_pins[i], OUTPUT);
      digitalWrite(denied_led_pins[i], LOW);
    }
  }
}

void door_loop() {
  unsigned long now = millis();
  for (byte i = 0; i < NUM_DOORS; i++) {
    boolean open = door_is_open(i);

    digitalWrite(strike_pins[i], open ? HIGH : LOW);
    if (accepted_led_pins[i] != 255) {
      digitalWrite(accepted_led_pins[i], open ? HIGH : LOW);
    }
    if (denied_led_pins[i] != 255) {
      digitalWrite(denied_led_pins[i], open ? HIGH : LOW);
    }
  }
}

void door_open(byte door_num) {
  if (door_num >= NUM_DOORS) {
    return;
  }
  
  hold_open[door_num].start = millis();
  hold_open[door_num].duration = strike_open_periods[door_num];
}

boolean door_is_open(byte door_num) {
  return INTERVAL_CONTAINS(hold_open[door_num], millis());
}


