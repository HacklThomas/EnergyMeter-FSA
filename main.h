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
	AUSLESEN = 2,
	SETTIME = 3,
	BLUETOOTHGRAPH = 4,
	IDLE = 5,
	DELETEFILES = 6,
	LISTFILES = 7,
	TOGGLECAN = 8,

	TEST = 20
} state_t;

typedef enum {
	BLUETOOTH = 0, USB = 1, UNDEFINED = 2
} interface_t;

typedef enum {
	FALSE = 0, TRUE = 1
} bool_t;

void init(void);
void sendData(char* data);
void saveData(void);
int receiveData(void);
int countFiles(void);
void setTime(void);
bool_t checkForStateChange(void);
void insertHeaderToFile(void);
void encrypt_cbc(uint8_t in[64]);
void sendCyphertext(void);
void sendCAN(void);
void initIVTMOD(void);
void sendFrameToIVTMOD(uint8_t db0, uint8_t db1, uint8_t db2, uint8_t db3);

// GLOBAL VARIABLES

state_t state;
int32_t current;
int32_t voltage;
int32_t temperature;

FATFS FatFs;
FIL fil;
DIR dj;
FILINFO fno;

interface_t interface;
volatile uint32_t timer;
volatile uint32_t oldtimerval;
uint8_t cyphertext[64];
bool_t canstate;
