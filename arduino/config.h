#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Enables serial debug output
#define DEBUG

// Hardware Note
//
// Dorbo was developed for a SparkFun Pro Micro (5V).  If you want to run it
// on another board, you'll probably have to change the way the interrupts
// are defined below.  Newer Arduino libraries include a digitalPinToInterrupt
// macro/function that you should use instead of INT0, INT1, INT2, etc.

//////////////////////////////////////////////////////////////////////////////
// Credential Storage
//////////////////////////////////////////////////////////////////////////////

// Number of Wiegand-26 credentials to store in EEPROM.  Each credential 
// requires 24-bits (3 bytes) of EEPROM.  We don't store the 2 parity bits.
// Increase the value if you have more memory or lower it if you need room 
// for other things.
#define WIEGAND26_MAX_CREDS 100

//////////////////////////////////////////////////////////////////////////////
// Wiegand Readers
//////////////////////////////////////////////////////////////////////////////

// Defines the number of Wiegand readers handled by this program.  Each reader 
// requires 2 data pins, and each pin must support external interrupts.
#define NUM_WIEGAND_READERS 2

// Pins connected to readers.  All pins must be capable of triggering
// external interrupts.  Must be an array initializer of two dimensions
// [NUM_WIEGAND_READERS][2] where the inner dimension defines 
// {DATA_0_PIN, DATA_1_PIN}.
#define WIEGAND_READER_PINS {{1, 0}, {2, 3}}

// Interrupts to hook for pins defined at WIEGAND_READER_PINS.  Must
// be an array initializer of the same size.
#define WIEGAND_READER_INTS {{INT2, INT3}, {INT1, INT0}}

// Abort the read and reset for a new value if reader input lines are idle 
// for this many milliseconds.  Prevents noise from accumulating and 
// spoiling the next read, which could be minutes or hours after the noise.  
// Typical inter-pulse spacing for readers is 1 ms, so a timeout a little 
// longer than this is useful.
// 
// If your environment has large amounts of electrical noise you could 
// try setting this to a smaller value, but don't make it shorter than 
// your reader's pulse period.
//
// If your reader uses an especially long period, adjust this to a larger
// value.  Longer values really won't hurt unless there's a lot of noise
// in the lines.
//
// If you don't have a physical Wiegand reader you can set this to a long
// value like 3000 (3 seconds), connect two normally-closed pushbuttons across
// the data lines to ground, and manually key in 26 bits.  Yes, this really 
// works.
//
// Must be greater than 0 and less than 2^31 (2147483648 ms ~= 24.9 days).
#define WIEGAND_INPUT_TIMEOUT_MS 10

//////////////////////////////////////////////////////////////////////////////
// Door Strikes and Indicators
//////////////////////////////////////////////////////////////////////////////

// Defines the number of doors controlled by this program.  Each door has one
// strike controlled by one output pin that goes HIGH to open the door, and
// LOW to close it.
#define NUM_DOORS 2

// Output pins connected to strikes.  Must be an array initializer of 
// length NUM_DOORS.
#define DOOR_STRIKE_PINS {8, 9}

// How many milliseconds to hold the strike open.  Must be an array 
// initializer of length NUM_DOORS.
#define DOOR_STRIKE_OPEN_PERIODS {5000, 5000}

// Output pins connected to "access granted" indicator LEDs.  Must be an 
// array initializer of length NUM_DOORs, each item is an array of 
// {pin, enabled_state} where enabled_state specifies the state (HIGH or LOW) 
// that the pin is set to to indicate success (reader devices differ in 
// their conventions).  Specify pin 255 with any enabled state to disable 
// the LED for that door index.
#define DOOR_ACCEPTED_LED_PINS   {{4, LOW}, {5, LOW}}

//////////////////////////////////////////////////////////////////////////////
// Status LED Panel
//////////////////////////////////////////////////////////////////////////////

// The LiquidCrystal constructor args for the pins connected to the
// ACM1602B LED module.
//
// LiquidCrystal(rs, enable, d4, d5, d6, d7)
// LiquidCrystal(rs, rw, enable, d4, d5, d6, d7)
// LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7)
// LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7) 
#define STATUS_PANEL_LIQUID_CRYSTAL_CONST_ARGS A3, A2, A1, A0, 15, 14

#endif

