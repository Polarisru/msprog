#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#ifdef __MINGW32__
#include <windows.h>
#endif

void msleep(uint32_t usec);

#endif
