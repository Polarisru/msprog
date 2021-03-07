#include "defines.h"
#include "ifaces.h"
#include "app.h"

#define SW_VER_NUMBER   "0.1"
#define SW_VER_DATE     "06.03.2021"

#define FILENAME_LEN    (64)
#define COMPORT_LEN     (32)

typedef struct
{
  bool      check;
  bool      write;
  bool      show_info;
  uint32_t  baudrate;
  int8_t    interface;
  char      port[COMPORT_LEN];
  char      file[FILENAME_LEN];
} tParam;

tParam parameters;

/** \brief Print help screen with list of commands
 *
 * \return Nothing
 *
 */
void help(void)
{
  uint8_t i;

  printf("  -b BAUDRATE  - set COM baudrate (default=115200)\n");
  printf("  -i INTERFACE - target interface\n");
  printf("  -c COM_PORT  - COM port to use (Win: COMx | *nix: /dev/ttyX)\n");
  printf("  -h           - show this help screen\n");
  printf("  -lX          - set logging level (0-all/1-warnings/2-errors)\n");
  printf("  -f FILE.HEX  - Hex file to write\n");
  printf("\n");
  printf("  List of supported interfaces:\n    ");
  for (i = 0; i < IFACES_GetNumber(); i++)
  {
    printf("%-14s", IFACES_GetNameByNumber(i));
    if ((i + 1) % 4 == 0 ? printf("\n    "): printf(""));
  }
  printf("\n");
}

/** \brief Main application function
 *
 * \param [in] argc Number of command line arguments
 * \param [in] argv Command line arguments
 * \return exit code for OS
 *
 */
int main(int argc, char* argv[])
{
  uint8_t i;
  uint8_t x;
  bool error;
  uint32_t tVal;
  char *pch;
  uint16_t val;

  printf("################################################################\n");
  printf("#   Simple command line interface for MultiServo programming   #\n");
  printf("#                    Ver. %3s (%00000010s)                     #\n", SW_VER_NUMBER, SW_VER_DATE);
  printf("################################################################\n\n");

  if (argc < 2)
  {
    help();
    return 0;
  }

  i = 1;
  while (i < argc)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
        case 'b':
          /**< set communication baudrate */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            if (sscanf(argv[i + 1], "%u", &tVal) == 1)
              parameters.baudrate = tVal;
            else
              printf("Baudrate parameter is wrong!\n");
          }
          break;
        case 'c':
          /**< set COM-port */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            strncpy(parameters.port, argv[i + 1], COMPORT_LEN);
            parameters.port[COMPORT_LEN - 1] = 0;
          } else
          {
            printf("COM-port name is missing!\n");
          }
          break;
        case 'd':
          /**< set interface */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            parameters.interface = IFACES_GetId(argv[i + 1]);
            if (parameters.interface < 0)
            {
              printf("Wrong or unsupported interface type: %s\n", argv[i + 1]);
              error = true;
            }
          }
          break;
        case 'f':
          /**< get file name */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            strncpy(parameters.file, argv[i + 1], FILENAME_LEN);
            parameters.file[FILENAME_LEN - 1] = 0;
            parameters.write = true;
            i++;
          } else
          {
            printf("%s: wrong file name for writing!\n", argv[i]);
            error = true;
          }
          break;
        case 'h':
          /**< print help screen */
          help();
          return 0;
        case 'm':
          /**< level of messaging */
          if (argv[i][2] >= '0' && argv[i][2] <= '2')
            LOG_SetLevel(argv[i][2] - '0');
          break;
        default:
          printf("Unknown parameter: %s\n", argv[i]);
          break;
      }
    }
    if (error == true)
      return -1;
  }

  if (parameters.interface < 0)
  {
    printf("Interface type (-i) is not set!\n");
    return -1;
  }
  if (strlen(parameters.port) == 0)
  {
    printf("COM port name is missing!\n");
    return -1;
  }
  if (!parameters.write && !parameters.check)
  {
    printf("Nothing to do, stopping\n");
    return -1;
  }

  APP_Execute();

  return 0;
}
