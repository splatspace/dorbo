#ifndef DOOR_H
#define DOOR_H

#include "config.h"

void door_init(void);
void door_loop(void);
void door_open(byte door_num);

boolean door_is_open(byte door_num);
long door_open_remaining(byte door_num);

#endif
