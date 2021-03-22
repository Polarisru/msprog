#ifndef XTEA_H
#define XTEA_H

#include <stdint.h>

#define XTEA_NUM_ROUNDS   	32

#define XTEA_BLOCK_LEN		8
#define XTEA_KEY_LEN		16

void XTEA_Encrypt(uint8_t *data, uint8_t *key, uint32_t len);
void XTEA_Decrypt(uint8_t *data, uint8_t *key, uint32_t len);

#endif
