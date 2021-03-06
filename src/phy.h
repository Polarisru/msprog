#ifndef PHY_H
#define PHY_H

#include "defines.h"

#define PHY_BAUDRATE      (115200)

bool PHY_Init(char *port, uint32_t baudrate, bool onDTR);
bool PHY_Send(uint8_t *data, uint16_t len);
bool PHY_Receive(uint8_t *data, uint16_t len);
bool PHY_ReceiveLine(char *line, char endl);
void PHY_Close(void);

#endif
