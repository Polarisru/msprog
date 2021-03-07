#include "app.h"
#include "ihex.h"
#include "log.h"
#include "phy.h"
#include "sleep.h"

#define FLASH_MAX_SIZE        (1024UL * 128)
#define FLASH_PAGE_SIZE       256

const char APP_CMD_Power[] = "PWR%1d";
const char APP_CMD_SetIface[] = "SI%d";
const char APP_CMD_SetBaudrate[] = "SB%d";
const char APP_CMD_SetId[] = "SID%d";
const char APP_CMD_StartBL[] = "BLS";
const char APP_CMD_StopBL[] = "BLQ";
const char APP_CMD_NL[] = "\n";

#define POWER_OFF_PAUSE_MS    500
#define POWER_ON_PAUSE_MS     50

bool APP_SendCmd(char *cmd)
{
  strcat(cmd, APP_CMD_NL);

  return false;
}

bool APP_Power(bool on)
{
  char str[32];

  sprintf(str, APP_CMD_Power, on);

  return APP_SendCmd(str);
}

bool APP_SetInterface(uint8_t iface)
{
  char str[32];

  sprintf(str, APP_CMD_SetIface, iface);

  return APP_SendCmd(str);
}

bool APP_SetBaudrate(uint32_t baudrate)
{
  char str[32];

  sprintf(str, APP_CMD_SetBaudrate, baudrate);

  return APP_SendCmd(str);
}

bool APP_OpenFile(char *filename, uint8_t *fdata)
{
  uint8_t errCode;
  FILE *fp;
  bool res = true;

  if ((fp = fopen(filename, "rt")) == NULL)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to open file: %s", filename);
    return false;
  }
  max_addr = 0;
  errCode = IHEX_ReadFile(fp, fdata, FLASH_MAX_SIZE, &max_addr);
  switch (errCode)
  {
    case IHEX_ERROR_FILE:
    case IHEX_ERROR_SIZE:
    case IHEX_ERROR_FMT:
    case IHEX_ERROR_CRC:
      LOG_Print(LOG_LEVEL_ERROR, "Problem reading Hex file");
      res = false;
      break;
    case IHEX_ERROR_NONE:
      break;
  }
  fclose(fp);

  return res;
}

bool APP_Execute(tParam *parameters)
{
  uint8_t *fdata;
  bool res = true;

  fdata = malloc(FLASH_MAX_SIZE);
  if (!fdata)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to allocate %d bytes\n", (int)FLASH_MAX_SIZE);
    return false;
  }

  while (1)
  {
    if (APP_OpenFile(parameters->file, fdata) == false)
    {
      res = false;
      break;
    }

    if (parameters->iface >= 0)
      APP_SetInterface((uint8_t)parameters->iface);

    if (parameters->baudrate != 0)
      APP_SetBaudrate((uint8_t)parameters->baudrate);

    APP_SetId(parameters->bus_id);

    APP_Power(false);
    msleep(POWER_OFF_PAUSE_MS);
    APP_Power(true);
    msleep(POWER_ON_PAUSE_MS);
    APP_SendCmd((char*)APP_CMD_StartBL);



    APP_SendCmd((char*)APP_CMD_StopBL);

    break;
  }

  free(fdata);

  return res;
}


