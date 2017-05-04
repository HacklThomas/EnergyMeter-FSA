
/*
http://msdn.microsoft.com/en-us/library/windows/desktop/aa363258%28v=vs.85%29.aspx

http://technet.microsoft.com/en-us/library/cc732236.aspx
*/

//------------------------------------
// INCLUDES
//------------------------------------
#include "serial.h"
#include "main.h"

//------------------------------------
// PRIVATE VARIABLES
//------------------------------------
HANDLE Cport[RS232_PORTNR];
char *comports[RS232_PORTNR]={"\\\\.\\COM1",  "\\\\.\\COM2",  "\\\\.\\COM3",  "\\\\.\\COM4",
                              "\\\\.\\COM5",  "\\\\.\\COM6",  "\\\\.\\COM7",  "\\\\.\\COM8",
                              "\\\\.\\COM9",  "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12",
                              "\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16"};
char mode_str[128];
static COMDevice comDevices[COM_MAXDEVICES];
static int noDevices = 0;
const char * comPtn = "COM???";

//------------------------------------
// GLOBAL FUNCTIONS
//------------------------------------
int RS232_OpenComport(int comport_number)
{
  if((comport_number>=RS232_PORTNR)||(comport_number<0))
  {
    printf("illegal comport number\n");
    return(1);
  }

  strcpy(mode_str, "baud=230400");
  strcat(mode_str, " data=8");
  strcat(mode_str, " parity=n");
  strcat(mode_str, " stop=1");

  strcat(mode_str, " dtr=on rts=on");

  Cport[comport_number] = CreateFileA(comports[comport_number],
                      GENERIC_READ|GENERIC_WRITE,
                      0,                          /* no share  */
                      NULL,                       /* no security */
                      OPEN_EXISTING,
                      0,                          /* no threads */
                      NULL);                      /* no templates */

  if(Cport[comport_number]==INVALID_HANDLE_VALUE)
  {
    printf("unable to open comport\n");
    return(1);
  }

  DCB port_settings;
  memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
  port_settings.DCBlength = sizeof(port_settings);

  if(!BuildCommDCBA(mode_str, &port_settings))
  {
    printf("unable to set comport dcb settings\n");
    CloseHandle(Cport[comport_number]);
    return(1);
  }

  if(!SetCommState(Cport[comport_number], &port_settings))
  {
    printf("unable to set comport cfg settings\n");
    CloseHandle(Cport[comport_number]);
    return(1);
  }

  COMMTIMEOUTS Cptimeouts;

  Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
  Cptimeouts.ReadTotalTimeoutMultiplier  = 0;
  Cptimeouts.ReadTotalTimeoutConstant    = 0;
  Cptimeouts.WriteTotalTimeoutMultiplier = 0;
  Cptimeouts.WriteTotalTimeoutConstant   = 0;

  if(!SetCommTimeouts(Cport[comport_number], &Cptimeouts))
  {
    printf("unable to set comport time-out settings\n");
    CloseHandle(Cport[comport_number]);
    return(1);
  }

  return(0);
}

int RS232_PollComport(int comport_number, unsigned char *buf, int size)
{
  int n;
  ReadFile(Cport[comport_number], buf, size, (LPDWORD)((void *)&n), NULL);
  return(n);
}

int RS232_SendByte(int comport_number, unsigned char byte)
{
  int n;
  WriteFile(Cport[comport_number], &byte, 1, (LPDWORD)((void *)&n), NULL);
  if(n<0)  return(1);
  return(0);
}

int RS232_SendBuf(int comport_number, unsigned char *buf, int size)
{
  int n;
  if(WriteFile(Cport[comport_number], buf, size, (LPDWORD)((void *)&n), NULL))
  {
    return(n);
  }
  return(-1);
}

void RS232_CloseComport(int comport_number)
{
  CloseHandle(Cport[comport_number]);
}

int RS232_IsDCDEnabled(int comport_number)
{
  int status;
  GetCommModemStatus(Cport[comport_number], (LPDWORD)((void *)&status));
  if(status&MS_RLSD_ON) return(1);
  else return(0);
}

int RS232_IsCTSEnabled(int comport_number)
{
  int status;
  GetCommModemStatus(Cport[comport_number], (LPDWORD)((void *)&status));
  if(status&MS_CTS_ON) return(1);
  else return(0);
}

int RS232_IsDSREnabled(int comport_number)
{
  int status;
  GetCommModemStatus(Cport[comport_number], (LPDWORD)((void *)&status));
  if(status&MS_DSR_ON) return(1);
  else return(0);
}

void RS232_enableDTR(int comport_number)
{
  EscapeCommFunction(Cport[comport_number], SETDTR);
}

void RS232_disableDTR(int comport_number)
{
  EscapeCommFunction(Cport[comport_number], CLRDTR);
}

void RS232_enableRTS(int comport_number)
{
  EscapeCommFunction(Cport[comport_number], SETRTS);
}

void RS232_disableRTS(int comport_number)
{
  EscapeCommFunction(Cport[comport_number], CLRRTS);
}

void RS232_flushRX(int comport_number)
{
  PurgeComm(Cport[comport_number], PURGE_RXCLEAR | PURGE_RXABORT);
}

void RS232_flushTX(int comport_number)
{
  PurgeComm(Cport[comport_number], PURGE_TXCLEAR | PURGE_TXABORT);
}

void RS232_flushRXTX(int comport_number)
{
  PurgeComm(Cport[comport_number], PURGE_RXCLEAR | PURGE_RXABORT);
  PurgeComm(Cport[comport_number], PURGE_TXCLEAR | PURGE_TXABORT);
}

void RS232_cputs(int comport_number, const char *text)  /* sends a string to serial port */
{
  while(*text != 0)   RS232_SendByte(comport_number, *(text++));
}

int RS232_GetPortnr(const char *devname)
{
  int i;

  char str[32];

#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
  strcpy(str, "/dev/");
#else  /* windows */
  strcpy(str, "\\\\.\\");
#endif
  strncat(str, devname, 16);
  str[31] = 0;

  for(i=0; i<RS232_PORTNR; i++)
  {
    if(!strcmp(comports[i], str))
    {
      return i;
    }
  }

  return -1;  /* device not found */
}
/*****************************************************************************/
int comEnumerate()
{
// Get devices information text
    size_t size = COM_MINDEVNAME;
    char * list = (char *) malloc(size);
    SetLastError(0);
    QueryDosDeviceA(NULL, list, size);
    while (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        size *= 2;
        char * nlist = realloc(list, size);
        if (!nlist) {
            free(list);
            return 0;
        }
        list = nlist;
        SetLastError(0);
        QueryDosDeviceA(NULL, list, size);
    }
// Gather all COM ports
    int port;
    const char * nlist = findPattern(list, comPtn, &port);
    noDevices = 0;
    while(port > 0 && noDevices < COM_MAXDEVICES) {
        COMDevice * com = &comDevices[noDevices ++];
        com->port = port;
        com->handle    = 0;
        nlist = findPattern(nlist, comPtn, &port);
    }
    free(list);
    return noDevices;
}

int comGetNoPorts()
{
    return noDevices;
}

const char * comGetPortName(int index)
{
    #define COM_MAXNAME    32
    static char name[COM_MAXNAME];
    if (index < 0 || index >= noDevices)
        return 0;
    sprintf(name, "COM%i", comDevices[index].port);
    return name;
}

const char * findPattern(const char * string, const char * pattern, int * value)
{
    char c, n = 0;
    const char * sp = string;
    const char * pp = pattern;
// Check for the string pattern
    while (1) {
        c = *sp ++;
        if (c == '\0') {
            if (*pp == '?') break;
            if (*sp == '\0') break;
            n = 0;
            pp = pattern;
        }else{
            if (*pp == '?') {
            // Expect a digit
                if (c >= '0' && c <= '9') {
                    n = n * 10 + (c - '0');
                    if (*pp ++ == '\0') break;
                }else{
                    n = 0;
                    pp = comPtn;
                }
            }else{
            // Expect a character
                if (c == *pp) {
                    if (*pp ++ == '\0') break;
                }else{
                    n = 0;
                    pp = comPtn;
                }
            }
        }
    }
// Return the value
    * value = n;
    return sp;
}


