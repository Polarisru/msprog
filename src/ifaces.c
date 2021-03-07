#include "ifaces.h"

tInterface IFACES_List[] =
{
  {"PWM",   0},
  {"RS485", 1},
  {"CAN",   2},
};

static int8_t IFACE_Id = IFACE_UNKNOWN_ID;

/** \brief Get device ID from name string
 *
 * \param [in] name Name to find as string
 * \return index of the found device or -1 as error
 *
 */
int8_t IFACES_GetId(char *name)
{
  uint8_t i;

  for (i = 0; i < sizeof(IFACES_List) / sizeof(tInterface); i++)
  {
    if (strcmp(name, IFACES_List[i].name) == 0)
    {
      IFACE_Id = i;
      return i;
    }
  }

  IFACE_Id = IFACE_UNKNOWN_ID;
  return -1;
}

/** \brief Get number of devices in the list
 *
 * \return Number of the devices as uint8_t
 *
 */
uint8_t IFACES_GetNumber(void)
{
  return (uint8_t)(sizeof(IFACES_List) / sizeof(tInterface));
}

/** \brief Get string name of the device by ID
 *
 * \return Device name as string
 *
 */
char *IFACES_GetNameByNumber(uint8_t number)
{
  if (number >= sizeof(IFACES_List) / sizeof(tInterface))
    number = 0;
  return IFACES_List[number].name;
}
