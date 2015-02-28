// Provides time since the microcontroller started as a struct that overflows after
// 136 years.

#ifndef TIME_H
#define TIME_H

#include <Arduino.h>
#include <limits.h>

#include "dorbo_utils.h"

// The range of struct time is:
//
// range(era) * range(millis) ms == (2^8) * (2^32) ms == 1099511627776 ms ~= 34.8 years
//
struct time {
  // An era is a period of 2^32 ms, about 49.7 days, which happens to be 
  // the range of the system's millis() counter.  Era 0 starts when the
  // microcontroller is turned on and ends after 2^32 ms have passed.
  // Era 1 starts immediately after era 0 ends, when millis() reads 0 again.
  uint8_t era;
  
  // Milliseconds since the start of the era.
  uint32_t millis;
};

#define TIME_CP(dst,src) memcpy(&dst, &src, sizeof(struct time))

#define P_TIME(t) \
  P("time<era="); \
  PDEC((t).era); \
  P(",ms="); \
  PDEC((t).millis); \
  P(">");

// Test if struct time t1 is less than struct time t2
#define TIME_LT_TIME(t1,t2) \
  ( \
    ((t1).era < (t2).era) \
    || ((t1).era == (t2).era && (t1).millis < (t2).millis) \
  )

// Test if struct time t1 is greater than struct time t2
#define TIME_GT_TIME(t1,t2) \
  ( \
    ((t1).era > (t2).era) \
    || ((t1).era == (t2).era && (t1).millis > (t2).millis) \
  )
  
void time_init();

// Updates loop_time and manages internal time state.  This must be called
// every loop (and probably first).
void time_loop();

// Puts the time of the start of era 0 in the specified time structure.
inline void time_epoch(struct time * time);

// Puts the current time (via millis()) into the specified time structure.
inline void time_now(struct time * time);

// Adds millis to the specified time structure.
inline void time_add(struct time * time, uint32_t millis);

//////////////////////////////////////////////////////////////////////////////
// Global Variable Definitions
//////////////////////////////////////////////////////////////////////////////

// Updated by time_loop()
extern struct time loop_time;

// Keeps the system's millis value from the last time_now call so we can 
// detect roll-over.
extern uint32_t last_now;

// Contains the current era (incremented when roll-over is detected).
extern byte current_era;

//////////////////////////////////////////////////////////////////////////////
// Inline Implementations
//////////////////////////////////////////////////////////////////////////////

inline void time_init() {
  // Reset the values so the current millis() epoch is the "beginning of history".
  time_epoch(&loop_time);
}

inline void time_loop() {
  time_now(&loop_time);
}

inline void time_epoch(struct time * time) {
  time->era = 0;
  time->millis = 0;
}

inline void time_now(struct time * time) {
  unsigned long now = millis();

  // If time jumped backwards, the system clock rolled over (happens every 49 days).
  if (now < last_now) {
    current_era++;
  }
  last_now = now;

  time->era = current_era;
  time->millis = now;
}

inline void time_add(struct time * time, uint32_t millis) {
  uint32_t millis_remaining_in_era = 0xffffffff - time->millis;
  if (millis > millis_remaining_in_era) {
    // Handle millis overflow by carrying 1 into the era.
    time->era++;
    time->millis = millis - millis_remaining_in_era;
  } else {
    // No overflow, straight addition.
    time->millis += millis;
  }
}

#endif
