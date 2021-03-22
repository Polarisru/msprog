#include "xtea.h"

static uint32_t rol(uint32_t base, uint32_t shift)
{
	uint32_t res;

  /* only 5 bits of shift are significant */
  shift &= 0x1F;
  res = (base << shift) | (base >> (32 - shift));
  return res;
}

/** \brief Encrypt one XTEA block
 *
 * \param [in] v Pointer to data block to encrypt
 * \param [in] k Pointer to encryption key
 * \return Nothing, Data block is used for output
 *
 */
static void XTEA_EncryptBlock(uint32_t *v, uint32_t const *k)
{
	uint8_t i;
	uint32_t y, z, sum = 0, delta = 0x9E3779B9;
	/* load and pre-white the registers */
	y = v[0] + k[0];
	z = v[1] + k[1];
	/* Round functions */
	for (i = 0; i < XTEA_NUM_ROUNDS; i++)
	{
		y += ((z << 4) ^ (z >> 5)) + (z ^ sum) + rol(k[sum & 3], z);
		sum += delta;
		z += ((y << 4) ^ (y >> 5)) + (y ^ sum) + rol(k[(sum >> 11) & 3], y);
	}
	/* post-white and store registers */
	v[0] = y ^ k[2];
	v[1] = z ^ k[3];
}

/** \brief Decrypt one XTEA block
 *
 * \param [in] v Pointer to data block to decrypt
 * \param [in] k Pointer to decryption key
 * \return Nothing, Data block is used for output
 *
 */
static void XTEA_DecryptBlock(uint32_t *v, uint32_t const *k)
{
	uint8_t i;
	uint32_t y, z, delta = 0x9E3779B9, sum = delta * XTEA_NUM_ROUNDS;

	z = v[1] ^ k[3];
	y = v[0] ^ k[2];
	for (i = 0; i < XTEA_NUM_ROUNDS; i++)
	{
		z -= ((y << 4) ^ (y >> 5)) + (y ^ sum) + rol(k[(sum >> 11) & 3], y);
		sum -= delta;
		y -= ((z << 4) ^ (z >> 5)) + (z ^ sum) + rol(k[sum & 3], z);

	}
	v[1] = z - k[1];
	v[0] = y - k[0];
}

/** \brief Encrypt data buffer (divided into XTEA blocks)
 *
 * \param [in] data Pointer to data buffer to encrypt
 * \param [in] key Pointer to encryption key
 * \param [in] len Length of data buffer
 * \return Nothing, Data buffer is used for output
 *
 */
void XTEA_Encrypt(uint8_t *data, uint8_t *key, uint32_t len)
{
	uint8_t i = 0;

	if (len == 0)
		return;

	len = ((len - 1) / XTEA_BLOCK_LEN + 1) * XTEA_BLOCK_LEN;

	while (len > 0)
	{
		key[XTEA_KEY_LEN - 1] = i++;
		XTEA_EncryptBlock((uint32_t*)data, (uint32_t*)key);
		data += XTEA_BLOCK_LEN;
		len -= XTEA_BLOCK_LEN;
	}
}

/** \brief Decrypt data buffer (divided into XTEA blocks)
 *
 * \param [in] data Pointer to data buffer to decrypt
 * \param [in] key Pointer to decryption key
 * \param [in] len Length of data buffer
 * \return Nothing, Data buffer is used for output
 *
 */
void XTEA_Decrypt(uint8_t *data, uint8_t *key, uint32_t len)
{
	uint8_t i = 0;

	if (len == 0)
		return;

	len = ((len - 1) / XTEA_BLOCK_LEN + 1) * XTEA_BLOCK_LEN;

	while (len > 0)
	{
		key[XTEA_KEY_LEN - 1] = i++;
		XTEA_DecryptBlock((uint32_t*)data, (uint32_t*)key);
		data += XTEA_BLOCK_LEN;
		len -= XTEA_BLOCK_LEN;
	}
}
