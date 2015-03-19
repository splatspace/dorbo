#include <Arduino.h>
#include "door.h"
#include "time.h"

// Declared and defined by config.h
static byte strike_pins[NUM_DOORS] = DOOR_STRIKE_PINS;
static byte accepted_led_pins[NUM_DOORS] = DOOR_ACCEPTED_LED_PINS;
static byte denied_led_pins[NUM_DOORS] = DOOR_DENIED_LED_PINS;
static unsigned int strike_open_periods[NUM_DOORS] = DOOR_STRIKE_OPEN_PERIODS;

static struct time close_at[NUM_DOORS];

void door_init(void) {
  for (byte i = 0; i < NUM_DOORS; i++) {
    // Initialize the time to 0 so the doors are initially closed
    time_epoch(&close_at[i]);

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
  
  // Copy the current loop time in
  TIME_CP(close_at[door_num], loop_time);
  
  // Add the configured open time
  time_add(&close_at[door_num], strike_open_periods[door_num]);
}

boolean door_is_open(byte door_num) {
  return TIME_LT_TIME(loop_time, close_at[door_num]);
}


