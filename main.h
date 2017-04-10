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

uint16_t crc16_update(uint16_t crc, uint8_t a);
int32_t interpret24bitAsInt32(uint8_t data1, uint8_t data2, uint8_t data3);
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
void UartHC06Init();
void initIVTMOD();
void sendFrameToIVTMOD(uint8_t db0, uint8_t db1, uint8_t db2, uint8_t db3);

state_t state;
interface_t interface;
FATFS FatFs;
FIL fil;
DIR dj;
FILINFO fno;

volatile uint32_t timer;
volatile uint32_t oldtimerval;
bool_t measureTemperature;
volatile int ivtConfig;

int32_t current;
int32_t voltage;
int32_t temperature;

char buffer[64];
uint8_t cyphertext[64];
uint8_t measureout[9];
volatile uint8_t measurein[9];
volatile uint8_t configout[9];
volatile uint8_t warmstartout[9];
volatile uint8_t error;

int savecounter;

volatile uint32_t candata[8];
volatile uint32_t cancounter;
bool_t canactive;
bool_t prevcanstate;
uint8_t errorbits;
uint32_t interrupttimer;
uint32_t savedatatimer;
uint32_t lastcantimer;

volatile uint32_t temperaturecounter;

