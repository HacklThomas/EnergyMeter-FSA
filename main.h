#include "stm32f4xx.h"
#include "tm_stm32f4_usb_vcp.h"
#include "tm_stm32f4_spi.h"
#include "tm_stm32f4_fatfs.h"
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_adc.h"
#include "tm_stm32f4_id.h"
#include "tm_stm32f4_rtc.h"
#include "stm32_ub_led.h"
#include "stm32_ub_uart.h"
#include "stm32_ub_can1.h"
#include "stm32_ub_can2.h"
#include "stm32_ub_systick.h"
#include "aes.h"
#include "rtc.h"
#include "defines.h"
#include "attributes.h"
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
uint8_t cyphertext[64];
char buffer[64];
bool_t canstate;


