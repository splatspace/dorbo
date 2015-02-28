#!/usr/bin/env python
#
# Manages the Arduino access controller over a serial port.
#
# Requires pySerial
import logging
import sys

from access_controller import AccessController, Wiegand26Credential


if len(sys.argv) != 3:
    print('usage: %s <device> <speed>' % sys.argv[0])
    sys.exit(1)

device = sys.argv[1]
speed = int(sys.argv[2])

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger()

with AccessController(device, speed) as ac:
    #ac.clear_wiegand26()
    ac.set_wiegand26(7, Wiegand26Credential(facility=44, user=12312))
    print(ac.get_wiegand26(7))
    print(ac.list_wiegand26())
