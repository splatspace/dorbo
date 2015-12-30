"""
Classes and types that provide for communication with the Dorbo access
controller hardware via RS-232 serial.
"""
import logging
from collections import namedtuple

import serial

Wiegand26Credential = namedtuple('Wiegand26Credential', ['facility', 'user'])
"""Represents one stored door access credential."""

SERIAL_ENCODING = 'utf8'
"""Encoding used to communicate with the access controller."""


class ReadTimeoutError(Exception):
    """
    Raised when a response from the access controller was not received in time.
    """
    pass

class ProtocolError(Exception):
    """
    Raised when the access controller's response violated the expected
    protocol or could not be understood.
    """
    pass


class AccessController(object):
    """
    Provides high- and low-level interfaces for communicating with the Dorbo
    access controller via RS-232 serial port.

    The controller is intended to be used as a context manager, like:

    with AccessController(serial_dev='/dev/ttyACM0') as controller:
        credentials = controller.list_wiegand26()
        controller.execute('l w26')

    """

    def __init__(self, serial_dev, serial_speed=115200):
        """

        :param serial_dev: the serial device the controller is attached to;
            this is a file like '/dev/ttyACM0' on POSIX systems, and is
            probably a string like "COM1" on Windows systems
        :param serial_speed: the speed at which to communicate to the access
            controller (115200 is standard)
        """
        self.logger = logging.getLogger(type(self).__name__)
        self.serial_kwargs = dict(port=serial_dev, baudrate=serial_speed,
                                  timeout=5)

        # Managed via context manager functions
        self.serial = None

    def __enter__(self):
        assert self.serial is None, 'the controller may not be re-entered'
        self.serial = serial.Serial(**self.serial_kwargs)
        self._shake_hands()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.serial.close()
        self.serial = None

    def clear_wiegand26(self):
        """
        Sets all stored Wiegand-26 credentials to facility 0, user 0.

        :raises ProtocolError: if there was an error communicating with the
            access controller
        """
        success, lines = self.execute('x w26')
        if not success:
            raise ProtocolError('Error clearing credentials: %s'
                                % ','.join(lines))

    def list_wiegand26(self):
        """
        List all stored Wiegand-26 credentials stored in the access controller.

        :return: a list of all Wiegand-26 credentials stored in the access
            controller with list indices corresponding to the controller
            storage index
        :raises ProtocolError: if there was an error communicating with the
            access controller
        """
        success, lines = self.execute('l w26')
        if not success:
            raise ProtocolError('Error listing credentials: %s'
                                % ','.join(lines))
        credentials = []
        for line in lines:
            fields = line.split()
            if len(fields) != 3:
                raise ProtocolError('Got %d fields instead of 3' % len(fields))
            credentials.append(Wiegand26Credential(facility=int(fields[1]),
                                                   user=int(fields[2])))
        return credentials

    def get_wiegand26(self, index):
        """
        Get a stored Wiegand-26 credential by index.

        :param index: the index of the credential to get
        :return: the Wiegand26Credential
        :raises ProtocolError: if there was an error communicating with the
            access controller
        """
        success, lines = self.execute('r w26 %d' % index)
        if not success:
            raise ProtocolError('Error getting credentials: %s'
                                % ','.join(lines))
        if len(lines) > 1:
            raise ProtocolError('Expected only one response line')
        fields = lines[0].split()
        if len(fields) != 3:
            raise ProtocolError('Got %d fields instead of 3' % len(fields))
        return Wiegand26Credential(facility=int(fields[1]), user=int(fields[2]))

    def set_wiegand26(self, index, wiegand26_credential):
        """
        Set a stored Wiegand-26 credential by index.

        :param index: the index of the credential to set
        :param wiegand26_credential: the credential data to set
        :raises ProtocolError: if there was an error communicating with the
            access controller
        """
        success, lines = self.execute('w w26 %d %d %d'
                                      % (index, wiegand26_credential.facility,
                                         wiegand26_credential.user))
        if not success:
            raise ProtocolError('Error setting credentials: %s'
                                % ','.join(lines))

    def execute(self, command):
        """
        Executes the command on the access controller.

        :param command: the command as a string
        :return: a tuple of (True, [result_lines]) when the command was
            successful, or (False, [result_lines]) if the command failed
        :raises ReadTimeoutError: if the command was not acknowledged in time
        """
        assert self.serial, 'can only execute inside a context manager'

        # Consume the text prompt or any left-overs from previous commands
        while self.serial.inWaiting() > 0:
            junk = self.serial.read()
            self.logger.debug('discarding pre-execution junk: %s', junk.strip())

        # Communication with the access controller uses very simple flow
        # control that's human friendly for manual debugging.  The host sends
        # a command terminated by a newline, the command executes, and the
        # controller's response is one or more lines that always ends in a full
        # line of text of "ok" or "err".
        command_bytes = bytes(command + '\n', SERIAL_ENCODING)
        self.serial.write(command_bytes)
        self.logger.debug('write: %r', command_bytes)
        self.serial.flush()

        result_lines = []
        while True:
            line_bytes = self.serial.readline()
            self.logger.debug('read: %r', line_bytes)
            line = str(line_bytes, SERIAL_ENCODING)
            if not line:
                self.logger.debug('timeout waiting for line: %r)', command_bytes)
                raise ReadTimeoutError('Timeout waiting for line: %s' % command)

            line = line.strip()
            if line == 'ok':
                return True, result_lines
            elif line == 'err':
                return False, result_lines
            else:
                result_lines.append(line)

    def _shake_hands(self):
        """
        Connect to the controller and confirm it's alive and ready for
        commands.
        """
        try:
            # Try for 5 seconds with a short timeout so we can retry
            # quickly if we're out of sync.
            old_timeout = self.serial.timeout
            new_timeout = 0.1

            self.serial.timeout = new_timeout
            tries = int(5 / new_timeout)

            for attempt in range(tries):
                self.logger.debug('handshake attempt')
                # An empty command should elicit an "ok" response
                try:
                    success, lines = self.execute('')
                    if success:
                        self.logger.debug('handshake success')
                        return True
                    else:
                        # Must have been some left-over 'err' response from
                        # a previous command.  Keep trying.
                        self.logger.debug('handshake error')
                        pass
                except ReadTimeoutError:
                    self.logger.debug('handshake timeout')
            return False
        finally:
            self.serial.timeout = old_timeout
