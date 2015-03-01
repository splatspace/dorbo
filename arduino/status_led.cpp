#include "status_led.h"
#include "dorbo_utils.h"
#include "time.h"

void status_led_init() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void status_led_loop() {
  // The lowest 10 bits of the total millis encode 1024 ms, which is close enough to 
  // 1 second for heartbeat purposes.  Accurate division is very slow on the AVR, 
  // so this is a good trade-off.
  
  // Light the LED for 256 of the 1024 ms in our period.
  digitalWrite(STATUS_LED_PIN, (loop_time.millis & 0x3ff) < 256 ? HIGH : LOW);
}

