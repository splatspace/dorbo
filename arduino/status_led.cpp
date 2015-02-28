#include "status_led.h"
#include "door_utils.h"
#include "time.h"

void status_led_init() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void status_led_loop() {
  digitalWrite(STATUS_LED_PIN, loop_time.millis < 250 ? HIGH : LOW);
}

