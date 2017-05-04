//
//  serial.h
//  SerialTerminal
//
//  Created by Thomas Hackl on 21.03.17.
//  Copyright © 2017 Thomas Hackl. All rights reserved.
//

#ifndef TH_SERIAL_H
#define TH_SERIAL_H

//------------------------------------
// INCLUDES
//------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/file.h>
#include <errno.h>

#include <dirent.h>
#include <stdint.h>

//------------------------------------
// DEFINES
//------------------------------------
#define _DARWIN_C_SOURCE
#define __USE_MISC
#define __USE_SVID

//------------------------------------
// GLOBALE FUNKTIONEN
//------------------------------------
int RS232_OpenComport(int, char *);
int RS232_PollComport(int, unsigned char *, int);
int RS232_SendByte(int, unsigned char);
void RS232_CloseComport(int);
void RS232_cputs(int, const char *);

int comEnumerate();
int comGetNoPorts();
const char * comGetPortName(int index);


#endif // TH_SERIAL_H


