#include <unistd.h>
#include "com.h"
#include "log.h"
#include "phy.h"
#include "sleep.h"

/** \brief Initialize physical interface
 *
 * \param [in] port Port name as string
 * \param [in] baudrate Transmission baudrate
 * \param [in] onDTR True if using DTR for power
 * \return true if success
 *
 */
bool PHY_Init(char *port, uint32_t baudrate, bool onDTR)
{
  return COM_Open(port, baudrate, true, true);
}

/** \brief Send data to physical interface
 *
 * \param [in] data Buffer with data
 * \param [in] len Length of data buffer
 * \return true if success
 *
 */
bool PHY_Send(uint8_t *data, uint16_t len)
{
  COM_Write(data, len);
  // read echo
  COM_Read(data, len);

  return true;
}

/** \brief Receive data from physical interface to data buffer
 *
 * \param [out] data Data buffer to write data in
 * \param [in] len Length of data to be received
 * \return true if success
 *
 */
bool PHY_Receive(uint8_t *data, uint16_t len)
{
  int val = COM_Read(data, len);
  if ((val < 0) || (val != len))
    return false;
  return true;
}

/** \brief Close physical interface
 *
 * \return Nothing
 *
 */
void PHY_Close(void)
{
  COM_Close();
}
