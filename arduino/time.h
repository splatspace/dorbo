// Provides time since the microcontroller started as a struct that overflows after
// 136 years.

#ifndef TIME_H
#define TIME_H

#include <Arduino.h>
#include <limits.h>
#include <util/atomic.h>

#include "door_utils.h"

struct time {
  unsigned long seconds;
  unsigned int millis;
};

#define TIME_CP(dst,src) memcpy(&dst, &src, sizeof(struct time))

#define P_TIME(t) \
  P("time<seconds="); \
  PDEC((t).seconds); \
  P(",ms="); \
  PDEC((t).millis); \
  P(">");

// Test if struct time t is less than seconds s and millis m
#define TIME_LT(t,s,m) \
  ( \
    ((t).seconds < (s)) \
    || ((t).seconds == (s) && (t).millis < (m)) \
  )

// Test if struct time t is greater than seconds s and millis m
#define TIME_GT(t,s,m) \
  ( \
    ((t).seconds > (s)) \
    || ((t).seconds == (s) && (t).millis > (m)) \
  )

#define TIME_LT_TIME(t1,t2) TIME_LT((t1),(t2).seconds,(t2).millis)
#define TIME_GT_TIME(t1,t2) TIME_GT((t1),(t2).seconds,(t2).millis)

// Updated at the start of each loop for use by any non-ISR function.
extern struct time loop_time;

void time_init();
inline void time_get(struct time * time, unsigned long system_millis);
inline void time_add(struct time * time, unsigned long seconds, unsigned long millis);

//////////////////////////////////////////////////////////////////////////////
// Inline Implementations
//////////////////////////////////////////////////////////////////////////////

// Tracks the time that elapsed in previous epochs (times before millis()
// rolled over).
static volatile struct time pre_history;

// Tracks the last time the ISR ran so we can detect millis() rollover..
static volatile unsigned long last_int_millis;

// Detect a millis rollover since the last call to this function.  If no 
// rollover happened, simply make a note of the time.  Call this function
// regularly at least twice per millis() epoch (~49 days) for reliable 
// detection.  A timer interrupt configured by time_init() handles this
// so user code doesn't have to.
//
// Call with interupts disabled.
inline void handle_rollover(unsigned long now) {
  // If time jumped backwards, the system clock rolled over (happens every 49 days).
  if (now < last_int_millis) {
    // Add a whole rollover period's worth of seconds and fractional millis to 
    // the pre-history time.
    struct time t;
    t.seconds = pre_history.seconds;
    t.millis = pre_history.millis;
    time_add(&t, ULONG_MAX / 1000, ULONG_MAX % 1000);
    pre_history.seconds = t.seconds;
    pre_history.millis = t.millis;
  }
  last_int_millis = now;
}

inline void time_get(struct time * time, unsigned long system_millis) {
  // Interrupts must be disabled to atomically read the pre-historic time values.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Handle the case that the millis that were specified indicates that it recently
    // rolled over.  An interrupt handler calls this periodically, but we must check
    // one just before we return any value to guarantee that it's handled.
    handle_rollover(system_millis);
    time->seconds = pre_history.seconds;
    time->millis = pre_history.millis;
  }

  // We can pass in more than a second's worth of millis
  time_add(time, 0, system_millis);
}

inline void time_add(struct time * time, unsigned long seconds, unsigned long millis) {
  // Add whole seconds
  time->seconds += seconds;
 
  if (millis > 0) {
    // Add the struct's fractional millis to the parameter value so we can do 
    // one pass to break it down into seconds.
    millis += time->millis;
  
    // Division is really slow on the AVR but I don't know of an alternative that's
    // faster when millis gets large.
    time->seconds += millis / 1000;
    time->millis = millis % 1000;
  }
}

#endif
