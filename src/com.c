#ifdef __MINGW32__
#include <windows.h>
#include <winbase.h>
#endif
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <math.h>
#include "defines.h"

#ifdef __MINGW32__
static HANDLE hSerial;
#endif
#ifdef __linux
static int fd;
#endif

static uint32_t COM_Baudrate = 115200;

/** \brief Open COM port with settings
 *
 * \param [in] port Port name as string
 * \param [in] baudrate Port baudrate
 * \param [in] have_parity true if parity should be switched on
 * \param [in] two_stopbits true if 2 stop bits used
 * \return true if succeed
 *
 */
bool COM_Open(char *port, uint32_t baudrate, bool have_parity, bool two_stopbits)
{
  printf("Opening %s at %u baud\n", port, baudrate);
  COM_Baudrate = baudrate;
  #ifdef __MINGW32__
  char str[64];
  uint8_t multiplier;

  sprintf(str, "\\\\.\\%s", port);
  hSerial = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0,
                              NULL, OPEN_EXISTING, 0, NULL);
  if (hSerial == INVALID_HANDLE_VALUE)
    return false;
  DCB dcbSerialParams = { 0 }; // Initializing DCB structure
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  GetCommState(hSerial, &dcbSerialParams);
  dcbSerialParams.BaudRate = baudrate;  // Setting BaudRate
  dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
  if (two_stopbits == true)
    dcbSerialParams.StopBits = TWOSTOPBITS;
  else
    dcbSerialParams.StopBits = ONESTOPBIT;
  if (have_parity == true)
    dcbSerialParams.Parity   = EVENPARITY;  // Setting Parity = None
  else
    dcbSerialParams.Parity   = NOPARITY;
  dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
  dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
  dcbSerialParams.fBinary = TRUE;
  dcbSerialParams.fParity = FALSE;
  dcbSerialParams.fOutxCtsFlow = FALSE;
  dcbSerialParams.fOutxDsrFlow = FALSE;
  SetCommState(hSerial, &dcbSerialParams);
  COMMTIMEOUTS timeouts;
  multiplier = (uint8_t)ceil((float)100000 / baudrate);
  timeouts.ReadIntervalTimeout = 20 * multiplier;
  timeouts.ReadTotalTimeoutMultiplier = 1 * multiplier;
  timeouts.ReadTotalTimeoutConstant = 100 * multiplier;
  timeouts.WriteTotalTimeoutMultiplier = 1;
  timeouts.WriteTotalTimeoutConstant = 1;
  SetCommTimeouts(hSerial, &timeouts);
  #endif

  #ifdef __linux
  fd = open(port, O_RDWR | O_NOCTTY );
  if (fd <0)
    return false;
  struct termios SerialPortSettings;
  tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */
  /* Setting the Baud rate */
  switch (baudrate)
  {
    case 300:
      cfsetispeed(&SerialPortSettings, B300);
      cfsetospeed(&SerialPortSettings, B300);
      break;
    case 9600:
      cfsetispeed(&SerialPortSettings, B9600);
      cfsetospeed(&SerialPortSettings, B9600);
      break;
    case 38400:
      cfsetispeed(&SerialPortSettings, B38400);
      cfsetospeed(&SerialPortSettings, B38400);
      break;
    case 57600:
      cfsetispeed(&SerialPortSettings, B57600);
      cfsetospeed(&SerialPortSettings, B57600);
      break;
    default:
    case 115200:
      cfsetispeed(&SerialPortSettings, B115200);
      cfsetospeed(&SerialPortSettings, B115200);
      break;
  }
  cfmakeraw(&SerialPortSettings);           /* Set raw mode (special processing disabled) */
  if (have_parity == true)
    SerialPortSettings.c_cflag |= PARENB;   /* Enables the Parity Enable bit(PARENB) */
  else
    SerialPortSettings.c_cflag &= ~PARENB;  /* Disables the Parity Enable bit(PARENB), so No Parity */
  if (two_stopbits == true)
    SerialPortSettings.c_cflag |= CSTOPB;   /* CSTOPB = 2 Stop bits */
  else
    SerialPortSettings.c_cflag &= ~CSTOPB;  /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
  SerialPortSettings.c_cflag |= (CREAD | CLOCAL); /* Enable receiver,Ignore Modem Control lines       */
  SerialPortSettings.c_cc[VMIN]  = 0;            // read doesn't block
  SerialPortSettings.c_cc[VTIME] = 5;            // 0.1 seconds read timeout
  tcsetattr(fd, TCSANOW, &SerialPortSettings);  /* Set the attributes to the termios structure*/
  tcflush(fd, TCIFLUSH);
  #endif

  return true;
}

/** \brief Write data to COM port
 *
 * \param [in] data Data buffer for writing
 * \param [in] len Length of data buffer
 * \return 0 if everything Ok
 *
 */
int COM_Write(uint8_t *data, uint16_t len)
{
  #ifdef __MINGW32__
  DWORD dwBytesWritten = 0;
  if (!WriteFile(hSerial, data, len, &dwBytesWritten, NULL))
    return -1;
  #endif
  #ifdef __linux
  int iOut = write(fd, data, len);
  if (iOut < 0)
    return -1;
  #endif

  return 0;
}

/** \brief Read data from COM port
 *
 * \param [out] data Data buffer to read data in
 * \param [in] len Length of data to read
 * \return number of received bytes as int
 *
 */
int COM_Read(uint8_t *data, uint16_t len)
{
  #ifdef __MINGW32__
  DWORD dwBytesRead = 0;
  ReadFile(hSerial, data, len, &dwBytesRead, NULL);
  #endif
  #ifdef __linux
  int dwBytesRead = read(fd, data, len);
  if (dwBytesRead < 0)
    return -1;
  #endif

  return dwBytesRead;
}

/** \brief Calculate time for transmission with current baudrate
 *
 * \param [in] len Length of transmitted data
 * \return time in milliseconds as uint16_t
 *
 */
uint16_t COM_GetTransTime(uint16_t len)
{
  return (uint16_t)(len * 1000 * 11 / COM_Baudrate + 1);
}

#ifdef __MINGW32__
void COM_WaitForTransmit(void)
{
  COMSTAT rStat;
  DWORD nErr;
  do {
    ClearCommError(hSerial, &nErr, &rStat);
  } while (rStat.cbOutQue > 0);
}
#endif // __MINGW32__

/** \brief Close current COM port
 *
 * \return Nothing
 *
 */
void COM_Close(void)
{
  printf("Closing COM port\n");
  #ifdef __MINGW32__
  CloseHandle(hSerial);
  #endif
  #ifdef __linux
  close(fd);
  #endif
}
