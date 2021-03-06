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
  bool res = COM_Open(port, baudrate, false, false);
  if (res == true)
    LOG_Print(LOG_LEVEL_LAST, "Opened %s at %u baud", port, baudrate);
  else
    LOG_Print(LOG_LEVEL_ERROR, "Can't open port: %s", port);
  return res;
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
  return (val == len);
}

/** \brief Receive line with end character
 *
 * \param line Pointer to received line
 * \param end End character
 * \return True if succeed
 *
 */
bool PHY_ReceiveLine(char *line, char endl)
{
  char ch = 0;

  while (COM_Read((uint8_t*)&ch, 1) > 0)
  {
    if (ch == endl)
      break;
    *line++ = ch;
  }
  *line = 0;

  return (ch == endl);
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
