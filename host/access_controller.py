from collections import namedtuple
import logging

import serial


Wiegand26Credential = namedtuple('Wiegand26Credential', ['facility', 'user'])


class AccessController(object):
    def __init__(self, serial_dev, serial_speed):
        self._logger = logging.getLogger(type(self).__name__)
        self._serial_dev = serial_dev
        self._serial_speed = serial_speed

        self._serial = None

    def __enter__(self):
        self._serial = serial.Serial(self._serial_dev, self._serial_speed, timeout=5)
        self._shake_hands()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._serial.close()
        self._serial = None

    def clear_wiegand26(self):
        """
        Sets all Wiegand-26 credentials to facility 0, user 0.
        :raises ValueError: if there was an error clearing the credentials
        """
        success, lines = self._execute('x w26')
        if not success:
            raise ValueError('Error clearing credentials: %s' % ','.join(lines))

    def list_wiegand26(self):
        """
        List all Wiegand-26 credentials stored in the access controller.

        :return: a list of all Wiegand-26 credentials stored in the access controller with
            list indicies corresponding to the controller storage index
        :raises ValueError: if there was an error getting the credential
        """
        success, lines = self._execute('l w26')
        if not success:
            raise ValueError('Error listing credentials: %s' % ','.join(lines))
        credentials = []
        for line in lines:
            fields = line.split()
            if len(fields) != 3:
                raise ValueError('Got %d fields instead of 3' % len(fields))
            credentials.append(Wiegand26Credential(facility=int(fields[1]), user=int(fields[2])))
        return credentials

    def get_wiegand26(self, index):
        """
        Get a Wiegand-26 credential by index.

        :param index: the index of the credential to get
        :return: the Wiegand26Credential
        :raises ValueError: if there was an error getting the credential
        """
        success, lines = self._execute('r w26 %d' % index)
        if not success:
            raise ValueError('Error getting credentials: %s' % ','.join(lines))
        if len(lines) > 1:
            raise ValueError('Expected only one response line')
        fields = lines[0].split()
        if len(fields) != 3:
            raise ValueError('Got %d fields instead of 3' % len(fields))
        return Wiegand26Credential(facility=int(fields[1]), user=int(fields[2]))

    def set_wiegand26(self, index, wiegand26_credential):
        """
        Set a Wiegand-26 credential by index.

        :param index: the index of the credential to set
        :param wiegand26_credential: the credential data to set
        :raises ValueError: if there was an error getting the credential
        """
        success, lines = self._execute('w w26 %d %d %d'
                                       % (index, wiegand26_credential.facility, wiegand26_credential.user))
        if not success:
            raise ValueError('Error setting credentials: %s' % ','.join(lines))

    def _shake_hands(self):
        """
        Connect to the controller and confirm it's alive.
        :return:
        """
        try:
            # Try for 5 seconds with a short timeout so we can
            old_timeout = self._serial.getTimeout()
            new_timeout = 0.1

            self._serial.setTimeout(new_timeout)
            tries = int(5 / new_timeout)

            for attempt in range(tries):
                self._logger.debug('handshake attempt')
                if self._execute(''):
                    self._logger.debug('handshake success')
                    return True
                self._logger.debug('handshake timeout')
            return False
        finally:
            self._serial.setTimeout(old_timeout)

    def _execute(self, command):
        """
        Executes the command on the access controller.

        :param command: the command
        :return: a tuple of (True, [result_lines]) when the command was successful, or
            (False, [result_lines]) if the command failed.
        """
        if not self._serial:
            raise ValueError('This method must be called only within a context manager')

        # Consume the text prompt or any left-overs from previous commands
        while self._serial.inWaiting() > 0:
            junk = self._serial.read()
            self._logger.debug('discarding junk: %s', junk.strip())

        # Communication with the access controller uses very simple flow control that's
        # human friendly for manual debugging.  The host sends a command terminated
        # by a newline, the command executes, and the Arduino's response is one
        # or more lines that always ends in a full line of text of "ok" or "err".
        self._serial.write(command + '\n')
        self._logger.debug('write line: %s', command)
        self._serial.flush()

        result_lines = []
        while True:
            line = self._serial.readline()
            self._logger.debug('read line: %s', line.strip())
            if not line:
                self._logger.debug('timeout waiting for ok ("%s")', command)
                return False

            line = line.strip()
            if line == 'ok':
                return True, result_lines
            elif line == 'err':
                return False, result_lines
            else:
                result_lines.append(line)
