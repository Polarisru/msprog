#include "app.h"
#include "phy.h"

const char APP_CMD_Power[] = "PWR%1d";
const char APP_CMD_SetIface[] = "SI%d";
const char APP_CMD_NL[] = "\n";

bool APP_SendCmd(char *cmd)
{
  strcat(cmd, APP_CMD_NL);

}

bool APP_Power(bool on)
{
  char str[32];

  sprintf(str, APP_CMD_Power, on);

  return APP_SendCmd(str);
}

bool APP_SetInterface(uint8_t interface)
{
  char str[32];

  sprintf(str, APP_CMD_SetIface, interface);

  return APP_SendCmd(str);
}

bool APP_Execute()
{

}


