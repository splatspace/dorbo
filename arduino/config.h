#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Enables serial debug output
#define DEBUG

//////////////////////////////////////////////////////////////////////////////
// Credential Storage
//////////////////////////////////////////////////////////////////////////////

// Number of Wiegand credentials to store in EEPROM.  Each credential 
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

// Enable one WIEGAND_PORT_n constant for the IO port that contains all the 
// pins your Wiegand readers are connected to.  The reader code enables Pin 
// Change Interrupt (PCI) handling on the specified port, but leaves the other 
// ports alone (so they can be used for other features that may not want to 
// use PCI).
//
// On the ATmega328 the IO ports are arranged like:
//   PORTB contains digital pin 8 to 13
//   PORTC contains analog input pins 0 to 5
//   PORTD contains digital pins 0 to 7
#define WIEGAND_PORTB
//#define WIEGAND_PORTC
//#define WIEGAND_PORTD

// Pins connected to readers.  All pins must be inside the IO port you 
// defined.
static byte wiegand_reader_pins[NUM_WIEGAND_READERS][2] = {{8, 9}, {10, 11}};

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
#define WIEGAND_INPUT_TIMEOUT_MS    10

//////////////////////////////////////////////////////////////////////////////
// Door Strikes
//////////////////////////////////////////////////////////////////////////////

// Defines the number of strikes activated by this program.  Each strike is 
// controlled by one digital output pin.
#define NUM_STRIKES 2

// Pins connected to strikes.
static byte strike_pins[NUM_STRIKES] = {6, 7};

// How many seconds to hold the strike open.
static unsigned int strike_open_periods[NUM_STRIKES] = {3, 3};

//////////////////////////////////////////////////////////////////////////////
// Misc Hardware
//////////////////////////////////////////////////////////////////////////////

// Blink a heartbeat on this pin so we know the controller loop is active.
#define STATUS_LED_PIN  13

#endif

