#include "stm32f4xx.h"
#include "th_usb_vcp.h"
#include "th_spi.h"
#include "th_fatfs.h"
#include "th_gpio.h"
#include "th_adc.h"
#include "th_crc.h"
#include "th_id.h"
#include "th_led.h"
#include "th_uart.h"
#include "th_can1.h"
#include "th_can2.h"
#include "th_systick.h"
#include "th_rtc.h"
#include "th_aes.h"
#include "th_rng.h"
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------
// HV-Schwelle ab der geloggt wird (mV)
#define MINVOLTAGE	5000

#define ENCRYPTION
#define DELETECODE 3528
#define TIMECODE 4827

typedef enum {
	INITIALISIEREN = 0,
	LOGGING = 1,
	MENU = 2,
	AUSLESEN = 3,
	SETTIME = 4,
	SETCAN = 5,
	BLUETOOTHGRAPH = 6,
	IDLE = 7,
	DELETEFILES = 8,
	LISTFILES = 9,
	TOGGLECAN = 10,
	DELETEINITFILE = 11,
	SETBTNAME = 12,

	TEST = 20
} state_t;

typedef enum {
	BLUETOOTH = 0, USB = 1, UNDEFINED = 2
} interface_t;

typedef enum {
	FALSE = 0, TRUE = 1
} bool_t;

void init();
void sendData(char* data);
void saveData();
int receiveData();
int countFiles();
void setTime();
bool_t checkForStateChange();
void insertHeaderToFile();
void encrypt_cbc(uint32_t input);
void sendCyphertext();
void sendZeilen();
void sendCAN();
void initIVTMOD();
void sendFrameToIVTMOD(uint8_t db0, uint8_t db1, uint8_t db2, uint8_t db3);
void setBTName();

state_t state;
int32_t current;
int32_t voltage;
int32_t temperature;

interface_t interface;
FATFS FatFs;
FIL fil;
DIR dj;
FILINFO fno;
FIL initfile;

volatile uint32_t timer;
volatile uint32_t blink;
volatile uint32_t interrupttimer;
uint8_t cyphertext[16];
char buffer[64];
char buffer2[64];
bool_t canstate;
bool_t hcreceive;
bool_t ivterror;
bool_t carderror;
bool_t syncing;

