
#ifndef TH_SERIAL_H
#define TH_SERIAL_H

//------------------------------------
// INCLUDES
//------------------------------------
#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>

//------------------------------------
// DEFINES / TYPEDEFS
//------------------------------------
#define RS232_PORTNR  16
#define _DARWIN_C_SOURCE
#define __USE_MISC
#define __USE_SVID
#define COM_MAXDEVICES 64
#define COM_MINDEVNAME 16384

typedef struct {
    int port;
    void * handle;
} COMDevice;

//------------------------------------
// GLOBAL FUNCTIONS
//------------------------------------
int RS232_OpenComport(int);
int RS232_PollComport(int, unsigned char *, int);
int RS232_SendByte(int, unsigned char);
int RS232_SendBuf(int, unsigned char *, int);
void RS232_CloseComport(int);
void RS232_cputs(int, const char *);
int RS232_IsDCDEnabled(int);
int RS232_IsCTSEnabled(int);
int RS232_IsDSREnabled(int);
void RS232_enableDTR(int);
void RS232_disableDTR(int);
void RS232_enableRTS(int);
void RS232_disableRTS(int);
void RS232_flushRX(int);
void RS232_flushTX(int);
void RS232_flushRXTX(int);
int RS232_GetPortnr(const char *);

int comEnumerate();
int comGetNoPorts();
const char * comGetPortName(int index);
const char * findPattern(const char * string, const char * pattern, int * value);

#endif // TH_SERIAL_H


