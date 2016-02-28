#!/usr/bin/env python3
#
# Reads door access fobs from a Google Spreadsheet and prints the fob numbers
# that are enabled to standard output, one fob number per line like
# "123 12345" (facility and user separated by a space).
#
# Requires: gspread oauth2client

import re
import sys
from argparse import ArgumentParser

import gspread
import oauth2client.client
from gspread.exceptions import SpreadsheetNotFound, WorksheetNotFound
from oauth2client.service_account import ServiceAccountCredentials

# Spreadsheet column headers, compared case-sensitive.
FOB_NUMBER_COLUMN = 'Fob Number'
ENABLED_COLUMN = 'Enabled'

FOB_NUMBER_PATTERN = re.compile('^(?P<facility>[0-9]{1,3})-'
                                '(?P<user>[0-9]{1,5})$')
ENABLED_PATTERN = re.compile('^TRUE$', flags=re.IGNORECASE)


def main():
    parser = ArgumentParser(
        description='Reads door access fobs from a Google Spreadsheet and '
                    'prints the ones that are enabled to standard output')

    parser.add_argument('--verbose', '-v',
                        help='enable verbose output to standard error',
                        action='store_true')
    parser.add_argument('sa_creds_file',
                        metavar='credentials-file.json',
                        help='JSON file containing Google service account '
                             'credentials')
    parser.add_argument('doc_name',
                        metavar='spreadsheet',
                        help='name of the Google Spreadsheets document')
    parser.add_argument('sheet_name',
                        metavar='worksheet',
                        help='name of the worksheet inside the spreadsheet '
                             'document that contains the fob rows')

    args = parser.parse_args()

    scopes = ['https://spreadsheets.google.com/feeds']
    try:
        credentials = ServiceAccountCredentials.from_json_keyfile_name(
            args.sa_creds_file, scopes)
        if args.verbose:
            sys.stderr.write('Service credentials loaded from "%s"\n'
                             % args.sa_creds_file)

        conn = gspread.authorize(credentials)
        if args.verbose:
            sys.stderr.write('Connected to Google Drive\n')
    except oauth2client.client.Error as e:
        sys.stderr.write('Authentication error: %s\n' % str(e))
        sys.stderr.write('Verify that the service account credentials file '
                         '"%s" contains valid access credentials.\n'
                         % args.sa_creds_file)
        sys.exit(2)

    try:
        doc = conn.open(args.doc_name)
        if args.verbose:
            sys.stderr.write('Spreadsheet document "%s" opened\n'
                             % args.doc_name)

        worksheet = doc.worksheet(args.sheet_name)
        if args.verbose:
            sys.stderr.write('Worksheet "%s" opened\n' % args.sheet_name)
    except SpreadsheetNotFound:
        sys.stderr.write('Spreadsheet document "%s" not found.\n'
                         % args.doc_name)
        sys.exit(3)
    except WorksheetNotFound:
        sys.stderr.write('Worksheet "%s" not found in spreadsheet "%s".\n'
                         % (args.sheet_name, args.doc_name))
        sys.exit(4)

    records = worksheet.get_all_records(head=1)
    if args.verbose:
        sys.stderr.write('%d records in worksheet\n' % len(records))

    # Validate records and collect decimal fob numbers.
    for record in records:
        for column_name in [FOB_NUMBER_COLUMN, ENABLED_COLUMN]:
            if column_name not in record:
                sys.stderr.write('Column "%s" not found in worksheet "%s".  '
                                 'Verify the worksheet contains the column '
                                 'with the correct name and capitalization.\n'
                                 % (column_name, args.sheet_name))
                sys.exit(5)

        if ENABLED_PATTERN.match(record[ENABLED_COLUMN]):
            # Parse the fob number into the facility and user parts
            fob_number = record[FOB_NUMBER_COLUMN]
            fob_match = FOB_NUMBER_PATTERN.match(fob_number)
            if fob_match:
                # Convert to int to removing leading zeroes
                facility = int(fob_match.group('facility'))
                user = int(fob_match.group('user'))
                if facility > 255:
                    sys.stderr.write('Ignoring invalid "facility" part of '
                                     'fob number: %s\n' % record)
                elif user > 65536:
                    sys.stderr.write('Ignoring invalid "user" part of '
                                     'fob number: %s\n' % record)
                else:
                    sys.stdout.write('%s %s\n' % (facility, user))
            else:
                sys.stderr.write('Ignoring enabled record with invalid fob '
                                 'number: %s\n' % record)
        elif args.verbose:
            sys.stderr.write('Ignoring non-enabled record: %s\n' % record)


if __name__ == '__main__':
    main()
