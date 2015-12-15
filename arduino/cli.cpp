#include "cli.h"
#include "wiegand.h"
#include "storage.h"
#include "door.h"

//////////////////////////////////////////////////////////////////////////////
// Command Line Interface Syntax
//////////////////////////////////////////////////////////////////////////////
//
// Read Credential
//
// "r <type-str> <index-dec>"
// 
// "r w26 3"            Read a 26-bit Wiegand credential from index 3.
//                      w26 codes contain only 24-bits of identity information
//                      (2 bits of parity are used on the wire).
//
// Output: 
//
// w26: "<index> <facility> <user>"
//
//////////////////////////////////////////////////////////////////////////////
//
// Write Credential
//
// "w <type-str> <index-dec> {type-specific}"
//
// "w w26 3 103 26441"  Write a 26-bit Wiegand credential to index 3 with 
//                      decimal facility code 103, decimal user code 26441
//                      w26 codes contain only 24-bits of identity information
//                      (2 bits of parity are used on the wire).
// "w x99 125 99123141" Write a 32-bit (hypothetical) Wizard-99 credential 
//                      type to index 125 with decimal data "99123141"
//
//////////////////////////////////////////////////////////////////////////////
//
// List Credentials
//
// "l <type-str>"
//
// "l w26"              List 26-bit Wiegand credentials in all indexes
//                      w26 codes contain only 24-bits of identity information
//                      (2 bits of parity are used on the wire).
//
// Output: 
//
// w26: "<index> <facility> <user>"...
//
//////////////////////////////////////////////////////////////////////////////
//
// Clear Credentials
//
// "x <type-str>"
//
// "x w26"              Writes a 26-bit Wiegand credential of facility code 0, 
//                      user code 0, to all indexes
//
//////////////////////////////////////////////////////////////////////////////
//
// Get Storage Info
//
// "i"
//
// Output: 
//
// "w26 <max-w26-credentials>"
//////////////////////////////////////////////////////////////////////////////
//
// Open Doors
//
// "o <door_num>"

#define CMD_READ       "r"
#define CMD_WRITE      "w"
#define CMD_LIST       "l"
#define CMD_CLEAR      "x"
#define CMD_INFO       "i"
#define CMD_OPEN       "o"
#define CMD_HELP       "h"
#define CMD_HELP2      "help"

// A 26-bit Wiegand credential (24-bits in RAM because we don't store the 
// 2 parity bits).
#define CRED_TYPE_WIEGAND_26    0
#define CRED_NAME_WIEGAND_26    "w26"

//////////////////////////////////////////////////////////////////////////////
// Parse Utilities
//////////////////////////////////////////////////////////////////////////////

// Buffers a command until we parse and run it
static char command[40];
static byte command_index = 0;

void clear_command() {
  memset(&command, 0, sizeof(command));
  command_index = 0;
}

uint8_t parse_uint8(char * str, uint8_t * dest) {
  char * endptr = 0;
  uint8_t value = strtol(str, &endptr, 10);
  // The strtol man pages says this signifies success
  if (*str != 0 && *endptr == 0) {
    *dest = value;
    return 1;
  }
  return 0;
}

uint16_t parse_uint16(char * str, uint16_t * dest) {
  char * endptr = 0;
  uint16_t value = strtol(str, &endptr, 10);
  // The strtol man pages says this signifies success
  if (*str != 0 && *endptr == 0) {
    *dest = value;
    return 1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Error Strings
//////////////////////////////////////////////////////////////////////////////

static const char * e_invalid_command = "invalid command";
static const char * e_missing_type = "missing type";
static const char * e_invalid_type = "invalid type";
static const char * e_missing_index = "missing index";
static const char * e_invalid_index = "invalid index";
static const char * e_missing_data = "missing data";
static const char * e_not_enough_data = "not enough data";
static const char * e_index_too_large = "index too large";
static const char * e_invalid_facility = "missing facility";
static const char * e_missing_facility = "invalid facility";
static const char * e_invalid_user = "missing user";
static const char * e_missing_user = "invalid user";
static const char * e_missing_door = "missing door";
static const char * e_invalid_door = "invalid door";

//////////////////////////////////////////////////////////////////////////////
// Read
//////////////////////////////////////////////////////////////////////////////

boolean exec_read_w26(uint8_t index) {
  struct wiegand26_credential cred;
  if (!storage_read_wiegand26_credential(index, &cred)) {
    Serial.println(e_index_too_large);
    return false;
  }
  Serial.print(index);
  Serial.print(' ');
  Serial.print(cred.facility);
  Serial.print(' ');
  Serial.println(cred.user);
  return true;
}

boolean exec_read(char * tok) {
  char * arg;
  
  // Parse credental type
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_type);
    return false;
  }
  uint8_t type;
  if (strcmp(arg, CRED_NAME_WIEGAND_26) == 0) {
    type = CRED_TYPE_WIEGAND_26;
  } else {
    Serial.println(e_invalid_type);
    return false;
  }
  
  // Parse index
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_index);
    return false;
  }
  uint8_t index;
  if (!parse_uint8(arg, &index)) {
    Serial.println(e_invalid_index);
    return false;
  }

  // Let the specialized exec finish
  if (type == CRED_TYPE_WIEGAND_26) {
    return exec_read_w26(index);
  }
  
  // Should not get here
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// Write
//////////////////////////////////////////////////////////////////////////////

boolean exec_write_w26(uint8_t index, char * tok) {
  char * arg;
  struct wiegand26_credential cred;

  // Parse facility
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_facility);
    return false;
  }
  if (!parse_uint8(arg, &cred.facility)) {
    Serial.println(e_invalid_facility);
    return false;
  }
  
  // Parse user
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_user);
    return false;
  }
  if (!parse_uint16(arg, &cred.user)) {
    Serial.println(e_invalid_user);
    return false;
  }

  // Execute
  if (!storage_write_wiegand26_credential(index, &cred)) {
    Serial.println(e_index_too_large);
    return false;
  }
  
  return true;
}

boolean exec_write(char * tok) {
  char * arg;
  
  // Parse credental type
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_type);
    return false;
  }
  uint8_t type;
  if (strcmp(arg, CRED_NAME_WIEGAND_26) == 0) {
    type = CRED_TYPE_WIEGAND_26;
  } else {
    Serial.println(e_invalid_type);
    return false;
  }
  
  // Parse index
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_index);
    return false;
  }
  uint8_t index;
  if (!parse_uint8(arg, &index)) {
    Serial.println(e_invalid_index);
    return false;
  }

  // Let the specialized exec finish
  if (type == CRED_TYPE_WIEGAND_26) {
    return exec_write_w26(index, tok);
  }
  
  // Should not get here
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// Clear
//////////////////////////////////////////////////////////////////////////////

boolean exec_clear_w26() {
  struct wiegand26_credential cred;
  cred.user = 0;
  cred.facility = 0;
  
  for (byte i = 0; i < WIEGAND26_MAX_CREDS; i++) {
    if (!storage_write_wiegand26_credential(i, &cred)) {
      Serial.println(e_index_too_large);
      return false;
    }
  }
  return true;
}

boolean exec_clear(char * tok) {
  char * arg;
  
  // Parse credental type
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_type);
    return false;
  }
  uint8_t type;
  if (strcmp(arg, CRED_NAME_WIEGAND_26) == 0) {
    type = CRED_TYPE_WIEGAND_26;
  } else {
    Serial.println(e_invalid_type);
    return false;
  }
  
  // Let the specialized exec finish
  if (type == CRED_TYPE_WIEGAND_26) {
    return exec_clear_w26();
  }
  
  // Should not get here
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// List
//////////////////////////////////////////////////////////////////////////////

boolean exec_list_w26() {
  struct wiegand26_credential cred;
  cred.user = 0;
  cred.facility = 0;
  
  for (byte i = 0; i < WIEGAND26_MAX_CREDS; i++) {
    if (!exec_read_w26(i)) {
      // It printed the error
      return false;
    }
  }
  return true;
}

boolean exec_list(char * tok) {
  char * arg;
  
  // Parse credental type
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_type);
    return false;
  }
  uint8_t type;
  if (strcmp(arg, CRED_NAME_WIEGAND_26) == 0) {
    type = CRED_TYPE_WIEGAND_26;
  } else {
    Serial.println(e_invalid_type);
    return false;
  }
  
  // Let the specialized exec finish
  if (type == CRED_TYPE_WIEGAND_26) {
    return exec_list_w26();
  }
  
  // Should not get here
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// Info
//////////////////////////////////////////////////////////////////////////////

boolean exec_info(char * tok) {
  Serial.print("w26 ");
  Serial.println(WIEGAND26_MAX_CREDS);
  return true;
}

//////////////////////////////////////////////////////////////////////////////
// Open
//////////////////////////////////////////////////////////////////////////////

boolean exec_open(char * tok) {
  char * arg;
  
  // Parse door num
  arg = strtok_r(NULL, " ", &tok);
  if (arg == NULL) {
    Serial.println(e_missing_door);
    return false;
  }
  byte door_num = atoi(arg);
  if (door_num < 0 || door_num >= NUM_DOORS) {
    Serial.println(e_invalid_door);
    return false;
  }
  
  door_open(door_num);
  
  return true;
}


//////////////////////////////////////////////////////////////////////////////
// Help
//////////////////////////////////////////////////////////////////////////////

boolean exec_help(char * tok) {
  Serial.println("h|help");
  Serial.println("l type");
  Serial.println("r type idx");
  Serial.println("w type idx n...");
  Serial.println("x type");
  Serial.println("o door");
  Serial.print("types: ");
  Serial.println(CRED_NAME_WIEGAND_26);
  return true;
}

//////////////////////////////////////////////////////////////////////////////
// Command Dispatch
//////////////////////////////////////////////////////////////////////////////

void process_command() {
  boolean ok = false;
  char * tok = NULL;
  const char * command_name = strtok_r(command, " ", &tok);
  if (strlen(command) == 0) {
    ok = true;
  } else if (strcmp(command_name, CMD_READ) == 0) {
    ok = exec_read(tok);
  } else if (strcmp(command_name, CMD_WRITE) == 0) {
    ok = exec_write(tok);
  } else if (strcmp(command_name, CMD_LIST) == 0) {
    ok = exec_list(tok);
  } else if (strcmp(command_name, CMD_CLEAR) == 0) {
    ok = exec_clear(tok);
  } else if (strcmp(command_name, CMD_INFO) == 0) {
    ok = exec_info(tok);
  } else if (strcmp(command_name, CMD_OPEN) == 0) {
    ok = exec_open(tok);
  } else if (strcmp(command_name, CMD_HELP) == 0 
    || strcmp(command_name, CMD_HELP2) == 0) {
    ok = exec_help(tok);
  } else {
    Serial.println(e_invalid_command);
  }
  
  if (ok) {
    Serial.println("ok");
  } else {
    Serial.println("err");
  }
  Serial.flush();
}

//////////////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////////////

void cli_init() {
  clear_command();
}

void cli_loop() {
  while (Serial && Serial.available()) {
    char c = Serial.read();
    
    if (c != '\r' && c != '\n' && command_index < sizeof(command)) {
      command[command_index++] = c;
      continue;
    }
    
    if (c == '\r' || c == '\n') {
      command[command_index] = 0;
      process_command();
    } else {
      Serial.println("command too long");
      Serial.println("err");
    }
  
    clear_command();
  }
}

