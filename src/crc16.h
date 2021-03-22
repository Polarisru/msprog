#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

#define CRC16_INIT_VAL      0xffff

uint16_t CRC16_Calc(uint16_t init, uint8_t *data, uint16_t len);
uint16_t CRC16_CalcData(uint8_t *data, uint16_t len);
uint16_t CRC16_CalcCCITT(uint8_t *data, uint16_t len);

#endif
