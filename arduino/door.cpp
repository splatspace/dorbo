#include <Arduino.h>
#include "door.h"
#include "time.h"

// Declared and defined by config.h
static byte strike_pins[NUM_DOORS] = DOOR_STRIKE_PINS;
static byte accepted_led_pins[NUM_DOORS][2] = DOOR_ACCEPTED_LED_PINS;
static unsigned int strike_open_periods[NUM_DOORS] = DOOR_STRIKE_OPEN_PERIODS;

// Half-open intervals (start is open, end is closed)
static struct interval hold_open[NUM_DOORS];

void door_init(void) {
  for (byte i = 0; i < NUM_DOORS; i++) {
    // Initialize the intervals so the doors are initially closed
    hold_open[i].start = 0;
    hold_open[i].duration = 0;

    // LOW is strike closed (door locked), HIGH is strike open (door unlocked)
    pinMode(strike_pins[i], OUTPUT);
    digitalWrite(strike_pins[i], LOW);
    
    byte led_pin = accepted_led_pins[i][0];
    byte enabled_state = accepted_led_pins[i][1];
    if (led_pin != 255) {
      pinMode(led_pin, OUTPUT);
      digitalWrite(led_pin, !enabled_state);
    }
  }
}

void door_loop() {
  unsigned long now = millis();
  for (byte i = 0; i < NUM_DOORS; i++) {
    boolean open = door_open_remaining_ms(i) > 0;

    digitalWrite(strike_pins[i], open ? HIGH : LOW);

    byte led_pin = accepted_led_pins[i][0];
    byte enabled_state = accepted_led_pins[i][1];
    if (led_pin != 255) {
      digitalWrite(led_pin, open ? enabled_state : !enabled_state);
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

uint32_t door_open_remaining_ms(byte door_num) {
  return open_closed_interval_remaining(&hold_open[door_num], millis());
}

