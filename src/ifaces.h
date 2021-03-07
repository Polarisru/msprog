#ifndef IFACES_H
#define IFACES_H

#include "defines.h"

#define IFACES_NAME_LEN    (16)

#define IFACE_UNKNOWN_ID   (-1)

typedef struct
{
  char     name[IFACES_NAME_LEN];
  uint8_t  num_id;
} tInterface;

int8_t IFACES_GetId(char *name);
uint8_t IFACES_GetNumber(void);
char *IFACES_GetNameByNumber(uint8_t number);

#endif
