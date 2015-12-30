#!/usr/bin/env python3
#
# Manages the access controller over a serial port.
#
# Requires pySerial
import logging
import sys
from argparse import ArgumentParser

from access_controller import AccessController

log_level_choices = ['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG']


def main():
    parser = ArgumentParser(
        description='Manage the Dorbo Access Controller via serial connection')

    parser.add_argument('--log-level', help='sets the log level',
                        choices=log_level_choices, dest='log_level',
                        default='INFO')
    parser.add_argument('-d', '--device',
                        help='the serial device to use to communicate with the '
                             'controller',
                        default='/dev/ttyACM0')
    parser.add_argument('-s', '--speed',
                        help='the speed (bytes/second) to use to communicate '
                             'with the controller',
                        default='115200')

    args = parser.parse_args()

    logging.basicConfig(level=args.log_level)
    logger = logging.getLogger()
    device = args.device
    try:
        speed = int(args.speed)
    except ValueError:
        print('Speed %s is not an integer' % args.speed)
        sys.exit(1)

    with AccessController(device, speed) as ac:
        # ac.clear_wiegand26()
        # ac.set_wiegand26(7, Wiegand26Credential(facility=44, user=12312))
        # print(ac.get_wiegand26(0))
        print(ac.list_wiegand26())

if __name__ == '__main__':
    main()
