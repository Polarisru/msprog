#include "app.h"
#include "crc16.h"
#include "ihex.h"
#include "log.h"
#include "phy.h"
#include "progress.h"
#include "sleep.h"
#include "xtea.h"

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
const char APP_CMD_ESC[] = "\x1B";
const char APP_CMD_OK[] = "OK";

#define POWER_OFF_PAUSE_MS    500
#define POWER_ON_PAUSE_MS     50

#define APP_RETRIES           4

void APP_Send(char *data)
{
  PHY_Send((uint8_t*)data, strlen(data));
}

void APP_SendData(uint8_t *data, uint16_t len)
{
  PHY_Send(data, len);
}

bool APP_GetReply(void)
{
  char reply[32];

  if (PHY_ReceiveLine(reply, APP_CMD_NL[0]) == false)
    return false;
  if (strncmp(reply, APP_CMD_OK, strlen(APP_CMD_OK)) != 0)
    return false;

  return true;
}

bool APP_SendCmd(char *cmd, bool wait_reply)
{
  char str[32];
  char reply[32];

  strcpy(str, APP_CMD_ESC);
  strcat(str, cmd);
  strcat(str, APP_CMD_NL);
  PHY_Send((uint8_t*)str, strlen(str));
  if (wait_reply == false)
    return true;
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

  return APP_SendCmd(str, true);
}

bool APP_SetInterface(uint8_t iface)
{
  char str[32];

  sprintf(str, APP_CMD_SetIface, iface);

  return APP_SendCmd(str, true);
}

bool APP_SetBaudrate(uint32_t baudrate)
{
  char str[32];

  sprintf(str, APP_CMD_SetBaudrate, baudrate);

  return APP_SendCmd(str, true);
}

bool APP_SetId(uint8_t id)
{
  char str[32];

  sprintf(str, APP_CMD_SetId, id);

  return APP_SendCmd(str, true);
}

bool APP_WriteFlash(uint16_t page, uint8_t *data)
{
  char str[32];

  sprintf(str, APP_CMD_WriteBL, page);
  APP_SendCmd(str, false);
  msleep(1);
  APP_SendData(data, FLASH_PAGE_SIZE);
  return APP_GetReply();
}

bool APP_CheckFlash(uint16_t page, uint8_t *data, uint16_t len)
{
  char str[32];
  uint16_t crc;

  crc = CRC16_CalcData(data, len);
  sprintf(str, APP_CMD_CheckBL, page, crc);
  return APP_SendCmd(str, true);
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
  memset(fdata, 0, FLASH_MAX_SIZE);
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
  uint8_t errors;
  uint16_t pages;

  fdata = malloc(FLASH_MAX_SIZE);
  if (!fdata)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to allocate %d bytes\n", (int)FLASH_MAX_SIZE);
    return false;
  }

  while (1)
  {
    if (PHY_Init(parameters->port, 115200, false) == false)
      break;

    if (APP_OpenFile(parameters->file, fdata, &len) == false)
      break;
    pages = (len - 1) / FLASH_PAGE_SIZE + 1;

    if (parameters->iface >= 0)
    {
      if (APP_SetInterface((uint8_t)parameters->iface) == false)
      {
        LOG_Print(LOG_LEVEL_ERROR, "Unable to set interface: %d\n", parameters->iface);
        break;
      }
    }

    if (parameters->baudrate != 0)
    {
      if (APP_SetBaudrate(parameters->baudrate) == false)
      {
        LOG_Print(LOG_LEVEL_ERROR, "Unable to set baudrate: %d\n", parameters->baudrate);
        break;
      }
    }

    if (parameters->bus_id >= 0)
    {
      if (APP_SetId(parameters->bus_id) == false)
      {
        LOG_Print(LOG_LEVEL_ERROR, "Unable to set bus ID: %d\n", parameters->bus_id);
        break;
      }
    }

    msleep(POWER_ON_PAUSE_MS);
    if (APP_SendCmd((char*)APP_CMD_StartBL, true) == false)
    {
      LOG_Print(LOG_LEVEL_ERROR, "Unable to start bootloader\n");
      break;
    }

    if (parameters->write)
    {
      PROGRESS_Print(0, pages, "Writing  FW: ", '#');
      errors = 0;
      i = 0;
      while ((i < len) & (errors < APP_RETRIES))
      {
        msleep(1);
        if (APP_WriteFlash(i / FLASH_PAGE_SIZE, &fdata[i]) == false)
        {
          errors++;
          continue;
        }
        errors = 0;
        i += FLASH_PAGE_SIZE;
        PROGRESS_Print(i / FLASH_PAGE_SIZE, pages, "Writing  FW: ", '#');
        msleep(5);
      }
      if (errors >= APP_RETRIES)
      {
        PROGRESS_Break();
        LOG_Print(LOG_LEVEL_ERROR, "Problem flashing Hex file");
        res = false;
        break;
      }
    }

    if (parameters->check)
    {
      PROGRESS_Print(0, pages, "Checking FW: ", '#');
      errors = 0;
      i = 0;
      while ((i < len) & (errors < APP_RETRIES))
      {
        if (APP_CheckFlash(i / FLASH_PAGE_SIZE, &fdata[i], FLASH_PAGE_SIZE) == false)
        {
          errors++;
          msleep(1);
          continue;
        }
        errors = 0;
        i += FLASH_PAGE_SIZE;
        PROGRESS_Print(i / FLASH_PAGE_SIZE, pages, "Checking FW: ", '#');
      }
      if (errors >= APP_RETRIES)
      {
        PROGRESS_Break();
        LOG_Print(LOG_LEVEL_ERROR, "Hex file differs from the firmware");
        res = false;
        break;
      }
    }

    LOG_Print(LOG_LEVEL_LAST, "Successfully executed");

    break;
  }

  i = 0;
  while (i++ < APP_RETRIES)
  {
    if (APP_SendCmd((char*)APP_CMD_StopBL, true) == true)
      break;
  }

  free(fdata);

  return res;
}


