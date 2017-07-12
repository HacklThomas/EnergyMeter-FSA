//--------------------------------------------------------------
// File     : th_rtc.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef TH_RTC_H
#define TH_RTC_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rtc.h"


//--------------------------------------------------------------
// Status Register Defines
//--------------------------------------------------------------
#define  RTC_STATUS_REG      RTC_BKP_DR0  // Status Register
#define  RTC_STATUS_INIT_OK  0x35AC       // RTC initialisiert
#define  RTC_STATUS_TIME_OK  0xA3C5       // RTC eingestellt

//--------------------------------------------------------------
// Zahlendarstellung der RTC-Struktur
//--------------------------------------------------------------
typedef enum {
  RTC_DEC = 0,  // Dezimal [8,9,10,11,12 usw]
  RTC_HEX,      // Hex     [0x08,0x09,0x10,0x11,0x12 usw]
}RTC_FORMAT_t;

//--------------------------------------------------------------
// Status der RTC beim init
//--------------------------------------------------------------
typedef enum {
  RTC_UNDEFINED =0, // RTC war noch nicht initialisiert
  RTC_INIT_OK,      // RTC war schon initialisiert
  RTC_TIME_OK       // RTC war schon eingestellt
}RTC_STATUS_t;

//--------------------------------------------------------------
// Struktur der RTC
//--------------------------------------------------------------
typedef struct {
  RTC_STATUS_t status;
  uint8_t std;     // studen   [0...23]
  uint8_t min;     // minuten  [0...59]
  uint8_t sek;     // sekunden [0...59]
  uint8_t tag;     // tag      [1...31]
  uint8_t monat;   // monat    [1...12]
  uint8_t jahr;    // jahr     [0...99]
  uint8_t wotag;   // wochentag [1...7] 1=Montag
}RTC_t;
RTC_t TH_RTC;

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
RTC_STATUS_t TH_RTC_Init(void);
void TH_RTC_SetClock(RTC_FORMAT_t format);
void TH_RTC_GetClock(RTC_FORMAT_t format);

//--------------------------------------------------------------
#endif // TH_RTC_H
