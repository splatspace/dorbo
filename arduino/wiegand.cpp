#include <util/atomic.h>

#include "wiegand.h"
#include "dorbo_utils.h"
#include "time.h"

// Define the PCI enablement and mask registers for the chosen port.
#if defined(WIEGAND_PORTB)
# define PORT_PCI_REG   PCIE0
# define PORT_MSK_REG   PCMSK0
# define PORT_PCI_VECT  PCINT0_vect
# define PORT_PINS_REG  PINB
#elif defined(WIEGAND_PORTC)
# define PORT_PCI_REG   PCIE1
# define PORT_MSK_REG   PCMSK1
# define PORT_PCI_VECT  PCINT1_vect
# define PORT_PINS_REG  PINC
#elif defined(WIEGAND_PORTD)
# define PORT_PCI_REG   PCIE2
# define PORT_MSK_REG   PCMSK2
# define PORT_PCI_VECT  PCINT2_vect
# define PORT_PINS_REG  PIND
#endif

struct wiegand_reader {
  byte zero_pin;
  byte one_pin;
  
  // PCI masks (one bit set for the pin address within the port)
  byte zero_pin_mask;
  byte one_pin_mask;
  
  // The following volatile fields must be accessed in an ATOMIC_BLOCK()
  
  // Number of bits read
  volatile byte           count;
  // Value (bits read are pushed on the right)
  volatile uint32_t       bits;
  // Millis of the last bits change (rollover-save)
  volatile unsigned long  last_changed;
};

static byte wiegand_reader_pins[NUM_WIEGAND_READERS][2] = WIEGAND_READER_PINS;

static struct wiegand_reader wiegand_readers[NUM_WIEGAND_READERS];

// The PCI ISR stashes the pins here to compare next time
static volatile byte previous_port_pins = 0;

void wiegand_readers_init(void) {
  byte port_pin_mask = 0;
  for (byte i = 0; i < NUM_WIEGAND_READERS; i++) {
    // Initialize all fields
    wiegand_readers[i].zero_pin = wiegand_reader_pins[i][0];
    wiegand_readers[i].one_pin = wiegand_reader_pins[i][1];
    wiegand_readers[i].count = 0;
    wiegand_readers[i].bits = 0;
    wiegand_readers[i].last_changed = millis();

    // Define the pins as inputs and enable the internal pull-up resistors.
    pinMode(wiegand_readers[i].zero_pin, INPUT_PULLUP);
    pinMode(wiegand_readers[i].one_pin, INPUT_PULLUP);

    // Map pins to the correct bits for the PCI ISR mask.
    wiegand_readers[i].zero_pin_mask = digitalPinToBitMask(wiegand_readers[i].zero_pin);
    wiegand_readers[i].one_pin_mask = digitalPinToBitMask(wiegand_readers[i].one_pin);
    
    port_pin_mask |= wiegand_readers[i].zero_pin_mask;
    port_pin_mask |= wiegand_readers[i].one_pin_mask;
  }
  
  // Enable pin change interrupts (PCI) on the port.
  sbi(PCICR, PORT_PCI_REG);

  // Set the mask to include all the declared pin masks
  PORT_MSK_REG |= port_pin_mask;

  // The idle state of the Wiegand data lines is HIGH (+5 volts), and the reader 
  // pulls the line LOW to signal 1 bit of credential data.  With the internal 
  // pull-up resistors active the pins read HIGH when the reader is idle (the 
  // circuit is open and current is flowing across the internal resistor) and the 
  // pins read LOW when the reader is signaling data (the reader closes the circuit
  // and allows current to flow to ground through the pin).
  //
  // Initialize the previous port pins to the "idle reader" state, which is each
  // pin HIGH, so we can correctly detect the first credential read.  The pin mask 
  // conveniently indicates which pins would be set in this state.
  previous_port_pins = port_pin_mask;
}

boolean wiegand_reader_get_wiegand26(byte reader_num, struct wiegand26_credential * cred) {
  struct wiegand_reader * reader = &wiegand_readers[reader_num];

  // Check if we have a complete credential
  byte count;
  uint32_t bits;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    count = reader->count;
    bits = reader->bits;
  }

  if (count < 26) {
    return false;
  }

  // Check for odd parity in the lower 13 bits and even parity in the upper 13 bits.
  boolean success = false;
  if ((__builtin_popcount(bits & 0x1fff) % 2) == 1
    && (__builtin_popcount((bits >> 13) & 0x1fff) % 2) == 0) {
    bits >>= 1;
    cred->user = bits & 0xffff;
    bits >>= 16;
    cred->facility = bits & 0xff;
    success = true;
  }
  
  // Reset for next time.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    reader->count = 0;
    reader->bits = 0;
    reader->last_changed = millis();
  }
  
  return success; 
}

// Typical Wiegand pulse period is 1 millisecond (with a pulse width of 
// 50 microseconds).  The ISR must complete before the next pulse comes.
ISR(PORT_PCI_VECT) {
  for (byte i = 0; i < NUM_WIEGAND_READERS; i++) {
    struct wiegand_reader * reader = &wiegand_readers[i];

    // If we don't have a 26-bit credential ready, check for changes on the "zero" and "one" 
    // data pins.  The initial "count" lets us avoid increasing count beyond 26, which means 
    // the value remains in the struct until the application code can read it out (or until 
    // an interrupt after the input timeout clears it out and starts over).
    //
    // Wiegand readers should not normally send us data faster than our application can
    // process it, but noisy transmission lines may cause random bits to trickle in.
    
    // Throw away incomplete credential data if it's been too long since the last 
    // bit was read.  This prevents noise in the lines from spoiling the next read.
    if (reader->count > 0 && reader->count < 26) {
      // Subtract to yield a signed difference, which will contain the correct
      // delta even if the system millis rolled over (so long as the delta is 
      // less than (2^32)/2 milliseconds).
      if ((long) (millis() - reader->last_changed) > WIEGAND_INPUT_TIMEOUT_MS) {
        reader->count = 0;
        reader->bits = 0;
        reader->last_changed = millis();
      }
    }
      
    // When the "zero" pin changes to LOW the device is sending us a 0
    if (reader->count < 26
        && (PORT_PINS_REG & reader->zero_pin_mask) != (previous_port_pins & reader->zero_pin_mask)
        && (PORT_PINS_REG & reader->zero_pin_mask) == reader->zero_pin_mask) {
      // Shift in a 0
      reader->count += 1;
      reader->bits <<= 1;
      reader->last_changed = millis();
    }
    
    // When the "one" pin changes to LOW the device is sending us a 1
    if (reader->count < 26
        && (PORT_PINS_REG & reader->one_pin_mask) != (previous_port_pins & reader->one_pin_mask) 
        && (PORT_PINS_REG & reader->one_pin_mask) == reader->one_pin_mask) {
      // Shift and add a 1
      reader->count += 1;
      reader->bits <<= 1;
      reader->bits |= 1;
      reader->last_changed = millis();
    }  
  }
  previous_port_pins = PORT_PINS_REG;
}

