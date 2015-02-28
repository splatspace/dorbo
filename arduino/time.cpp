#include <limits.h>

#include "time.h"
#include "door_utils.h"

// Define a long interrupt period (every few seconds is fine) to detect millis()
// rollover.
#if F_CPU == 16000000
// Runs every 4 seconds
# define TIME_INT_PRESCALE 1024
# define TIME_INT_COMPARE  62499
#else
# error Timer prescale and compare values not defined for your CPU
#endif

#if TIME_INT_PRESCALE == 1
# define CLOCK_TCCR1B_CSFLAGS     (bit(CS10))
#elif TIME_INT_PRESCALE == 8
# define CLOCK_TCCR1B_CSFLAGS     (bit(CS11))
#elif TIME_INT_PRESCALE == 64
# define CLOCK_TCCR1B_CSFLAGS     (bit(CS11) | bit(CS10))
#elif TIME_INT_PRESCALE == 256
# define CLOCK_TCCR1B_CSFLAGS     (bit(CS12))
#elif TIME_INT_PRESCALE == 1024
# define CLOCK_TCCR1B_CSFLAGS     (bit(CS12) | bit(CS10))
#else 
# error Unhandled prescale value.  Is this an AVR from the future?
#endif

struct time loop_time;

void time_init() {
  // Disable interrupts
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Enable CTC mode (WGM12) on 16-bit timer1 and set the flags to indicate 
    // the prescale.
    TCCR1A = 0;
    TCCR1B = bit(WGM12) | CLOCK_TCCR1B_CSFLAGS;
    // Set the compare value for one second
    OCR1A = TIME_INT_COMPARE;
    // Enable the interrupt
    TIMSK1 = bit(OCIE1A);
    
    // Reset the values so the current millis() epoch is the "beginning of history".
    pre_history.seconds = 0;
    pre_history.millis = 0;
    last_int_millis = millis();
  }
}

ISR(TIMER1_COMPA_vect) {
  // Detect whether the system millis() timer recently rolled over.
  handle_rollover(millis());
}


