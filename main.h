#include "stm32f4xx.h"
#include "th_usb_vcp.h"
#include "th_spi.h"
#include "th_fatfs.h"
#include "th_gpio.h"
#include "th_adc.h"
#include "th_id.h"
#include "th_led.h"
#include "th_uart.h"
#include "th_can1.h"
#include "th_can2.h"
#include "th_systick.h"
#include "th_rtc.h"
#include "th_aes.h"
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	SETHC = 12,

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
void encrypt_cbc();
void sendCyphertext();
void sendCAN();
void initIVTMOD();
void sendFrameToIVTMOD(uint8_t db0, uint8_t db1, uint8_t db2, uint8_t db3);

state_t state;
int32_t current;
int32_t voltage;
int32_t temperature;

interface_t interface;
FATFS FatFs;
FIL fil;
DIR dj;
FILINFO fno;

volatile uint32_t timer;
volatile uint32_t blink;
uint8_t cyphertext[64];
char buffer[64];
bool_t canstate;
bool_t hcreceive;


