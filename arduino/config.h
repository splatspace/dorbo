#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Enables serial debug output
#define DEBUG

//////////////////////////////////////////////////////////////////////////////
// Credential Storage
//////////////////////////////////////////////////////////////////////////////

// Number of Wiegand-26 credentials to store in EEPROM.  Each credential 
// requires 3 bytes (24-bits; we don't store the 2 parity bits) of EEPROM 
// and 3 bytes of SRAM (for buffering during load).  Feel free to raise if 
// you have more memory or lower if you need room for other things.
#define WIEGAND26_MAX_CREDS 100

//////////////////////////////////////////////////////////////////////////////
// Wiegand Readers
//////////////////////////////////////////////////////////////////////////////

// Defines the number of Wiegand readers handled by this program.  Each reader 
// requires 2 data pins.  All readers must be connected to pins in a single IO 
// port on the microcontroller.
#define NUM_WIEGAND_READERS 2

// Pins connected to readers.  All pins must be capable of triggering
// external interrupts.  Must be an array initializer of two dimensions
// [NUM_WIEGAND_READERS][2] where the inner dimension defines 
// {DATA_0_PIN, DATA_1_PIN}.
#define WIEGAND_READER_PINS {{0, 1},       {2, 3}}

// Interrupts to hook for pins defined at WIEGAND_READER_PINS.  Must
// be an array initializer of the same size.
#define WIEGAND_READER_INTS {{INT3, INT2}, {INT1, INT0}}

// Abort the read and reset for a new value if reader input lines are idle 
// for this many milliseconds).  Prevents noise from accumulating and 
// spoiling the next read.  Typical inter-pulse spacing for readers is
// 1 ms, so a short timeout a little longer than this is useful.
// 
// If your environment has large amounts of electrical noise you could 
// try setting this to a smaller value, but don't make it shorter than 
// your reader's pulse period.
//
// If your reader uses an especially long period, adjust this to a larger
// value.  Longer values really won't hurt unless there's a lot of noise
// in the lines.
//
// If you don't have a physical Wiegand reader you can set this to a value
// to 3000 (3 seconds), connect two normally-closed pushbuttons across the
// data lines to ground, and manually key in 26 bits.  Yes, this really 
// works.
//
// Must be greater than 0 and less than 2^31 (2147483648 ms ~= 24.9 days).
#define WIEGAND_INPUT_TIMEOUT_MS 10

//////////////////////////////////////////////////////////////////////////////
// Door Strikes and Indicators
//////////////////////////////////////////////////////////////////////////////

// Defines the number of doors controlled by this program.  Each door has one
// strike controlled by one output pin.
#define NUM_DOORS 2

// Output pins connected to strikes.  Must be an array initializer of 
// length NUM_DOORS.
#define DOOR_STRIKE_PINS {6, 7}

// How many milliseconds to hold the strike open.  Must be an array 
// initializer of length NUM_DOORS.
#define DOOR_STRIKE_OPEN_PERIODS {5000, 5000}

// Output pins connected to "access granted" indicator LEDs.  The pin is
// put HIGH while the strike is open to indicate the credentials were
// accepted.  Must be an array initializer of length NUM_DOORS.  
// Specify 255 to disable the feature for a strike index.
#define DOOR_ACCEPTED_LED_PINS {2, 3}

// Output pins connected to "access denied" indicator LEDs.  The pin is
// put HIGH while the strike is open to indicate the credentials were
// rejected.  Must be an array initializer of length NUM_DOORS.  
// Specify 255 to disable the feature for a strike index.
#define DOOR_DENIED_LED_PINS {4, 5}

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
#define STATUS_PANEL_LIQUID_CRYSTAL_CONST_ARGS 15, 14, A0, A1, A2, A3

#endif

