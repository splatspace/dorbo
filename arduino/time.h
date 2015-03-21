#ifndef TIME_H
#define TIME_H

// An interval of system millis.  The interval can be used as an open, closed
// or half-open interval.
//
// Rules:
// - An open or half-open interval with a duration of 0 specifies an instant
// - A closed interval with a duration of 0 is empty (contains no millis)
// - A negative duration describes an empty interval for all interval types
struct interval {
  // The millis value that defines the start of the interval
  uint32_t start;
  // The duration in ms that is added to the start to calculate the end of
  // the interval.
  uint32_t duration;
};

// start + duration safely overflows in the same fashion as millis() to
// yield the end of the duration.  When start is large and duration is large, 
// the addition can overflow to yield an end < than start.
//
// The difference of the end with t is the remaining duration.  When t <= end, 
// the subtraction does not underflow and the result is always <= duration.  
// When t > end underflow causes the result to be > duration.
#define INTERVAL_REMAINING(i,t) ((i).start + (i).duration) - (t)
  
inline uint32_t open_closed_interval_remaining(struct interval * i, uint32_t t) {
  uint32_t remaining = INTERVAL_REMAINING(*i, t);
  if (remaining <= i->duration) {
    // At the start of the interval, or inside it, but not yet at the end
    return remaining;
  } else {
    // At the end or outside the interval
    return 0;
  }
}

#endif TIME_H
