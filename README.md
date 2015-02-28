# Dorbo

Dorbo is an entry access control program for the Arduino and a host computer.
Itopens doors when contactless credendials (key fobs) are presented to
readers.Authorized credentials can be configured easily through the serial port
while it runs, and they are saved in the Arduino's EEPROM for maximum
reliability.

# Network Architecture

I love box art, so here's a picture:

                         ┌┄┄┄┄┄┄┄┄┄┄┄┄┄┄┐        ┌┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┐
                         ┆ ╔══════════╗ ┆        ┆ ╔═══════════════╗ ┆
    ╭──────────╮         ┆ ║ Dorbo    ║ ┆        ┆ ║ Dorbo Access  ║ ┆
    │ Internet │ ◀─https─▶ ║ Host     ║ ◀─rs-232─▶ ║ Controller    ║ ┆
    ╰──────────╯         ┆ ║ Software ║ ┆        ┆ ║ Software      ║ ┆
    (optional)           ┆ ╚══════════╝ ┆        ┆ ╚═══════════════╝ ┆
                         ┆              ┆        ┆                   ┆
                         ┆ PC-ish       ┆        ┆ Arduino-ish       ┆
                         └┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘        └┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘

## Host Software

The Dorbo host software runs on a computer that has an RS-232 compatible serial
port, can run Python 2.7, and (optionally) can connect to the Internet.  This
computer can be a PC, laptop, Raspberry Pi, an old SGI Indy, or whatever you
have lying around.  The Dorbo host software assumes the host computer is running
a Unix operating system, but the code is pure Python and could be adapted easily
to run on other operating systems.

The host software can be found in the "host" subdirectory in the repo.

## Access Controller Software

The Arduino software runs on an Atmel ATmega328-based microcontroller with an
RS-232 serial interface.  An Arduino Uno works well, but any similar model with
that as many pins and as much SRAM and EEPROM should also work.

Access credentials are stored in the microcontroller's EEPROM so the host
computer is not required to be available  each time credentials are presented to
the system.  This makes the system more reliable as the host computer is more
complex than the access controller and is more likely to fail.  Support for
external EEPROM storage could be added to the controller software in order to
increase the number of credentials that can be stored on it.

The access controller software can be found in the "arduino" subdirectory in
the repo.

## Known Limitations

The access controller does not have a real-time clock, but it uses an
internal timekeeping method that allows it to run for 34.8 years without clock
rollover.  This period is long enough we can ignore the effects of clock
rollover when doing clock math.  This simplifies the implementation of
several features.

If your access controller will be continuously powered for more than 34.8 years
and this period is unacceptably low, the easiest change you can make is to
change the type of the "era" field in the time structure to uint16_t.  This
will give you ~= 8919.6 years before rollover at the expense of slower time
arithmetic.
