//--------------------------------------------------------------
// File     : th_led.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : RTC / PWR / RCC
// Funktion : RTC - Real Time Clock
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_rtc.h"

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_RTC_Config(void);

//--------------------------------------------------------------
// GLobale Variabeln
//--------------------------------------------------------------
RTC_TimeTypeDef   RTC_TimeStructure; // muss Global sein
RTC_DateTypeDef   RTC_DateStructure; // muss Global sein
RTC_InitTypeDef   RTC_InitStructure; // muss Global sein

//--------------------------------------------------------------
// Init und Start der RTC
// Return Wert :
//  -> RTC_UNDEFINED = RTC war noch nicht initialisiert
//  -> RTC_INIT_OK   = RTC war schon initialisiert
//                     (aber noch nicht eingestellt)
//  -> RTC_TIME_OK   = RTC war schon eingestellt
//--------------------------------------------------------------
RTC_STATUS_t TH_RTC_Init(void)
{
  RTC_STATUS_t ret_wert=RTC_UNDEFINED;
  uint32_t status;

  // Clock enable fuer PWR
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  // Zugriff auf RTC freigeben
  PWR_BackupAccessCmd(ENABLE);

  // Test vom Status-Register
  status=RTC_ReadBackupRegister(RTC_STATUS_REG);
  if(status==RTC_STATUS_TIME_OK) {
    // wenn Uhrzeit schon eingestellt ist
    ret_wert=RTC_TIME_OK;
    TH_RTC.status=RTC_TIME_OK;

    // warte bis synconisiert
    RTC_WaitForSynchro();
    #if RTC_USE_WAKEUP_ISR==1
      RTC_ClearITPendingBit(RTC_IT_WUT);
      EXTI_ClearITPendingBit(EXTI_Line22);
    #endif

    // RTC auslesen
    TH_RTC_GetClock(RTC_DEC);
  }
  else if(status==RTC_STATUS_INIT_OK) {
    // wenn RTC schon initialisiert ist
    ret_wert=RTC_INIT_OK;
    TH_RTC.status=RTC_INIT_OK;

    // warte bis synconisiert
    RTC_WaitForSynchro();
    #if RTC_USE_WAKEUP_ISR==1
      RTC_ClearITPendingBit(RTC_IT_WUT);
      EXTI_ClearITPendingBit(EXTI_Line22);
    #endif

    // RTC auslesen
      TH_RTC_GetClock(RTC_DEC);
  }
  else {
    // wenn RTC noch nicht initialisiert ist
    ret_wert=RTC_UNDEFINED;
    TH_RTC.status=RTC_UNDEFINED;

    // RTC Konfig und start
    P_RTC_Config();

    // RTC zuruecksetzen auf (0:00:00 / 1.1.00)
    TH_RTC.std=0;
    TH_RTC.min=0;
    TH_RTC.sek=0;
    TH_RTC.tag=1;
    TH_RTC.monat=1;
    TH_RTC.jahr=0;
    TH_RTC.wotag=1;
    TH_RTC_SetClock(RTC_DEC);

    TH_RTC.status=RTC_INIT_OK;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// RTC setzen
// vor dem Aufruf muss die aktuelle Uhrzeit und das Datum
// in der Struktur "UB_RTC" gespeichert werden
// format : [RTC_DEC, RTC_HEX]
//--------------------------------------------------------------
void TH_RTC_SetClock(RTC_FORMAT_t format)
{
  // Check auf Min+Max
  if(TH_RTC.std>23) TH_RTC.std=23;
  if(TH_RTC.min>59) TH_RTC.min=59;
  if(TH_RTC.sek>59) TH_RTC.sek=59;

  if(TH_RTC.tag<1) TH_RTC.tag=1;
  if(TH_RTC.tag>31) TH_RTC.tag=31;
  if(TH_RTC.monat<1) TH_RTC.monat=1;
  if(TH_RTC.monat>12) TH_RTC.monat=12;
  if(TH_RTC.jahr>99) TH_RTC.jahr=99;
  if(TH_RTC.wotag<1) TH_RTC.wotag=1;
  if(TH_RTC.wotag>7) TH_RTC.wotag=7;

  // Zeit einstellen
  RTC_TimeStructure.RTC_Hours = TH_RTC.std;
  RTC_TimeStructure.RTC_Minutes = TH_RTC.min;
  RTC_TimeStructure.RTC_Seconds = TH_RTC.sek;
  if(format==RTC_DEC) {
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
  }
  else {
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
  }

  // Datum einstellen
  RTC_DateStructure.RTC_Date = TH_RTC.tag;
  RTC_DateStructure.RTC_Month = TH_RTC.monat;
  RTC_DateStructure.RTC_Year = TH_RTC.jahr;
  RTC_DateStructure.RTC_WeekDay = TH_RTC.wotag;
  if(format==RTC_DEC) {
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
  }
  else {
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
  }

  // Sonstige Settings
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv =  0xFF;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);

  // Status-Register beschreiben
  // falls Settings vom User gemacht wurden
  if(TH_RTC.status!=RTC_UNDEFINED) {
    RTC_WriteBackupRegister(RTC_STATUS_REG, RTC_STATUS_TIME_OK);
  }
}


//--------------------------------------------------------------
// RTC auslesen
// nach dem Aufruf steht die aktuelle Uhrzeit
// und das Datum in der Struktur "UB_RTC"
// format : [RTC_DEC, RTC_HEX]
//--------------------------------------------------------------
void TH_RTC_GetClock(RTC_FORMAT_t format)
{
  // Zeit auslesen
  if(format==RTC_DEC) {
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  }
  else {
    RTC_GetTime(RTC_Format_BCD, &RTC_TimeStructure);
  }
  TH_RTC.std = RTC_TimeStructure.RTC_Hours;
  TH_RTC.min = RTC_TimeStructure.RTC_Minutes;
  TH_RTC.sek = RTC_TimeStructure.RTC_Seconds;

  // Datum auslesen
  if(format==RTC_DEC) {
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  }
  else {
    RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);
  }
  TH_RTC.tag = RTC_DateStructure.RTC_Date;
  TH_RTC.monat = RTC_DateStructure.RTC_Month;
  TH_RTC.jahr = RTC_DateStructure.RTC_Year;
  TH_RTC.wotag = RTC_DateStructure.RTC_WeekDay;

}

//--------------------------------------------------------------
// interne Funktion
// RTC Settings einstellen
//--------------------------------------------------------------
void P_RTC_Config(void)
{
  // Clock auf LSE einstellen
  RCC_LSEConfig(RCC_LSE_ON);

  // warten bis Clock eingestellt
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

  // Clock enable fuer LSE
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  // enable RTC
  RCC_RTCCLKCmd(ENABLE);

  // warte bis synconisiert
  RTC_WaitForSynchro();

  // Status-Register beschreiben
  RTC_WriteBackupRegister(RTC_STATUS_REG, RTC_STATUS_INIT_OK);
}
