#ifndef DEFINES_H
#define DEFINES_H

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#define FILENAME_LEN    (64)
#define COMPORT_LEN     (32)

typedef struct
{
  bool      check;
  bool      write;
  bool      show_info;
  uint32_t  baudrate;
  int8_t    iface;
  uint8_t   bus_id;
  char      port[COMPORT_LEN];
  char      file[FILENAME_LEN];
} tParam;

#endif
