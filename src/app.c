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
const char APP_CMD_WriteBL[] = "BLF%02X";
const char APP_CMD_CheckBL[] = "BLC%02X:%04X";
const char APP_CMD_NL[] = "\n";
const char APP_CMD_OK[] = "OK";

#define POWER_OFF_PAUSE_MS    500
#define POWER_ON_PAUSE_MS     50

void APP_Send(char *data)
{
  PHY_Send((uint8_t*)data, strlen(data));
}

void APP_SendData(uint8_t *data, uint16_t len)
{
  PHY_Send(data, len);
}

bool APP_SendCmd(char *cmd)
{
  char str[32];
  char reply[32];

  strcpy(str, cmd);
  strcat(str, APP_CMD_NL);
  PHY_Send((uint8_t*)str, strlen(str));
  if (PHY_ReceiveLine(reply, APP_CMD_NL[0]) == false)
    return false;
  if (strncmp(reply, APP_CMD_OK, strlen(APP_CMD_OK)) != 0)
    return false;

  return true;
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

bool APP_SetId(uint8_t id)
{
  char str[32];

  sprintf(str, APP_CMD_SetId, id);

  return APP_SendCmd(str);
}

bool APP_WriteFlash(uint16_t page, uint8_t *data)
{
  char str[32];

  sprintf(str, APP_CMD_WriteBL, page);
  strcat(str, APP_CMD_NL);
  APP_Send(str);

  return true;
}

bool APP_OpenFile(char *filename, uint8_t *fdata, uint32_t *len)
{
  uint8_t errCode;
  FILE *fp;
  bool res = true;
  uint32_t max_addr;

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
      *len = max_addr;
      break;
  }
  fclose(fp);

  return res;
}

bool APP_Execute(tParam *parameters)
{
  uint8_t *fdata;
  bool res = true;
  uint32_t i, len;

  fdata = malloc(FLASH_MAX_SIZE);
  if (!fdata)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to allocate %d bytes\n", (int)FLASH_MAX_SIZE);
    return false;
  }

  while (1)
  {
    if (PHY_Init(parameters->port, 115200, false) == false)
      return false;

    if (APP_OpenFile(parameters->file, fdata, &len) == false)
    {
      res = false;
      break;
    }

    if (parameters->iface >= 0)
      APP_SetInterface((uint8_t)parameters->iface);

    if (parameters->baudrate != 0)
      APP_SetBaudrate(parameters->baudrate);

    APP_SetId(parameters->bus_id);

    //APP_Power(false);
    //msleep(POWER_OFF_PAUSE_MS);
    //APP_Power(true);
    msleep(POWER_ON_PAUSE_MS);
    APP_SendCmd((char*)APP_CMD_StartBL);

    if (parameters->write)
    {
      i = 0;
      while (i < len)
      {
        if (APP_WriteFlash(i / FLASH_PAGE_SIZE, fdata) == false)
        {
          continue;
        }
        APP_SendData(&fdata[i], FLASH_PAGE_SIZE);
        i += FLASH_PAGE_SIZE;
      }
    }

    if (parameters->check)
    {
      i = 0;
      while (i < len)
      {
        if (APP_CheckFlash(i / FLASH_PAGE_SIZE, fdata) == false)
        {
          continue;
        }
        APP_SendData(&fdata[i], FLASH_PAGE_SIZE);
        i += FLASH_PAGE_SIZE;
      }
    }

    APP_SendCmd((char*)APP_CMD_StopBL);

    break;
  }

  free(fdata);

  return res;
}


