// Indicates device status through LEDs
//

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

#include "config.h"
#include "time.h"

void status_led_init();
void status_led_loop();

#endif

