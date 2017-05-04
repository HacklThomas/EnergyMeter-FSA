//
//  serial.c
//  SerialTerminal
//
//  Created by Thomas Hackl on 21.03.17.
//  Copyright © 2017 Thomas Hackl. All rights reserved.
//

/*
http://pubs.opengroup.org/onlinepubs/7908799/xsh/termios.h.html
http://man7.org/linux/man-pages/man3/termios.3.html
http://man7.org/linux/man-pages/man4/tty_ioctl.4.html 
*/

//------------------------------------
// INCLUDES
//------------------------------------
#include "serial.h"
#include "main.h"

//------------------------------------
// DEFINES / TYPEDEFS
//------------------------------------
#define RS232_PORTNR  38
#define COM_MAXDEVICES        64

typedef struct {
    char * port;
    int handle;
} COMDevice;

//------------------------------------
// VARIABLEN
//------------------------------------
static COMDevice comDevices[COM_MAXDEVICES];
static int noDevices = 0;

#if defined(__APPLE__) && defined(__MACH__)
static const char * devBases[] = {
    "tty."
};
static int noBases = 1;
#else
static const char * devBases[] = {
    "ttyACM", "ttyUSB", "rfcomm", "ttyS"
};
static int noBases = 4;
#endif

int Cport[RS232_PORTNR],
    error;

struct termios new_port_settings,
       old_port_settings[RS232_PORTNR];

char *comports[RS232_PORTNR]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4","/dev/ttyS5",
                       "/dev/ttyS6","/dev/ttyS7","/dev/ttyS8","/dev/ttyS9","/dev/ttyS10","/dev/ttyS11",
                       "/dev/ttyS12","/dev/ttyS13","/dev/ttyS14","/dev/ttyS15","/dev/ttyUSB0",
                       "/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3","/dev/ttyUSB4","/dev/ttyUSB5",
                       "/dev/ttyAMA0","/dev/ttyAMA1","/dev/ttyACM0","/dev/ttyACM1",
                       "/dev/rfcomm0","/dev/rfcomm1","/dev/ircomm0","/dev/ircomm1",
                       "/dev/cuau0","/dev/cuau1","/dev/cuau2","/dev/cuau3",
                       "/dev/cuaU0","/dev/cuaU1","/dev/cuaU2","/dev/cuaU3"};

//------------------------------------
// PRIVATE FUNKTIONEN
//------------------------------------
void _AppendDevices(const char * base);

//------------------------------------
// GLOBALE FUNKTIONEN
//------------------------------------
int RS232_OpenComport(int comport_number, char *name)
{
    comports[0] = name;
    
  int status;

  if((comport_number>=RS232_PORTNR)||(comport_number<0))
  {
    printf("illegal comport number\n");
    return(1);
  }

  int baudr = B230400;
  
  int cbits=CS8,
      cpar=0,
      ipar=IGNPAR,
      bstop=0;

  Cport[comport_number] = open(comports[comport_number], O_RDWR | O_NOCTTY | O_NDELAY);
  if(Cport[comport_number]==-1)
  {
    perror("unable to open comport ");
    return(1);
  }

  /* lock access so that another process can't also use the port */
  if(flock(Cport[comport_number], LOCK_EX | LOCK_NB) != 0)
  {
    close(Cport[comport_number]);
    perror("Another process has locked the comport.");
    return(1);
  }

  error = tcgetattr(Cport[comport_number], old_port_settings + comport_number);
  if(error==-1)
  {
    close(Cport[comport_number]);
    flock(Cport[comport_number], LOCK_UN);  /* free the port so that others can use it. */
    perror("unable to read portsettings ");
    return(1);
  }
  memset(&new_port_settings, 0, sizeof(new_port_settings));  /* clear the new struct */

  new_port_settings.c_cflag = cbits | cpar | bstop | CLOCAL | CREAD;
  new_port_settings.c_iflag = ipar;
  new_port_settings.c_oflag = 0;
  new_port_settings.c_lflag = 0;
  new_port_settings.c_cc[VMIN] = 0;      /* block untill n bytes are received */
  new_port_settings.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */

  cfsetispeed(&new_port_settings, baudr);
  cfsetospeed(&new_port_settings, baudr);

  error = tcsetattr(Cport[comport_number], TCSANOW, &new_port_settings);
  if(error==-1)
  {
    tcsetattr(Cport[comport_number], TCSANOW, old_port_settings + comport_number);
    close(Cport[comport_number]);
    flock(Cport[comport_number], LOCK_UN);  /* free the port so that others can use it. */
    perror("unable to adjust portsettings ");
    return(1);
  }

  if(ioctl(Cport[comport_number], TIOCMGET, &status) == -1)
  {
    tcsetattr(Cport[comport_number], TCSANOW, old_port_settings + comport_number);
    flock(Cport[comport_number], LOCK_UN);  /* free the port so that others can use it. */
    perror("unable to get portstatus");
    return(1);
  }

  status |= TIOCM_DTR;    /* turn on DTR */
  status |= TIOCM_RTS;    /* turn on RTS */

  if(ioctl(Cport[comport_number], TIOCMSET, &status) == -1)
  {
    tcsetattr(Cport[comport_number], TCSANOW, old_port_settings + comport_number);
    flock(Cport[comport_number], LOCK_UN);  /* free the port so that others can use it. */
    perror("unable to set portstatus");
    return(1);
  }

  return(0);
}

int RS232_PollComport(int comport_number, unsigned char *buf, int size)
{
  int n;

  n = read(Cport[comport_number], buf, size);

  if(n < 0)
  {
    if(errno == EAGAIN)  return 0;
  }

  return(n);
}

int RS232_SendByte(int comport_number, unsigned char byte)
{
  int n = write(Cport[comport_number], &byte, 1);
  if(n < 0)
  {
    if(errno == EAGAIN)
    {
      return 0;
    }
    else
    {
      return 1;
    }
  }

  return(0);
}

void RS232_CloseComport(int comport_number)
{
  int status;

  if(ioctl(Cport[comport_number], TIOCMGET, &status) == -1)
  {
    perror("unable to get portstatus");
  }

  status &= ~TIOCM_DTR;    /* turn off DTR */
  status &= ~TIOCM_RTS;    /* turn off RTS */

  if(ioctl(Cport[comport_number], TIOCMSET, &status) == -1)
  {
    perror("unable to set portstatus");
  }

  tcsetattr(Cport[comport_number], TCSANOW, old_port_settings + comport_number);
  close(Cport[comport_number]);

  flock(Cport[comport_number], LOCK_UN);  /* free the port so that others can use it. */
}

void RS232_cputs(int comport_number, const char *text)  /* sends a string to serial port */
{
    while(*text != 0){
        RS232_SendByte(comport_number, *(text++));
    }
}

/*****************************************************************************/

int comEnumerate()
{
    for (int i = 0; i < noDevices; i++) {
        if (comDevices[i].port) free(comDevices[i].port);
        comDevices[i].port = NULL;
    }
    noDevices = 0;
    for (int i = 0; i < noBases; i++)
        _AppendDevices(devBases[i]);
    return noDevices;
}

int comGetNoPorts()
{
    return noDevices;
}

const char * comGetPortName(int index) {
    if (index >= noDevices || index < 0)
        return NULL;
    return comDevices[index].port;
}

void _AppendDevices(const char * base)
{
    int baseLen = strlen(base);
    struct dirent * dp;
    // Enumerate devices
    DIR * dirp = opendir("/dev");
    while ((dp = readdir(dirp)) && noDevices < COM_MAXDEVICES) {
        if (strlen(dp->d_name) >= baseLen) {
            if (memcmp(base, dp->d_name, baseLen) == 0) {
                COMDevice * com = &comDevices[noDevices ++];
                com->port = (char *) strdup(dp->d_name);
                com->handle = -1;
            }
        }
    }
    closedir(dirp);
}
