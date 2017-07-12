//--------------------------------------------------------------
// File     : th_systick.h
//--------------------------------------------------------------

#ifndef TH_SYSTICK_H
#define TH_SYSTICK_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"

extern __IO uint32_t TH_Time2;

//--------------------------------------------------------------
// Auflösung der Systick
//--------------------------------------------------------------
#define  SYSTICK_RESOLUTION   1    // 1us Auflösung

#define TH_DELAY_Time2()				(TH_Time2)
#define TH_DELAY_SetTime2(time)			(TH_Time2 = (time))

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void TH_SYSTICK_Init(void);
void TH_SYSTICK_Pause_us(volatile uint32_t pause);
void TH_SYSTICK_Pause_ms(volatile uint32_t pause);
void TH_SYSTICK_Pause_s(volatile uint32_t pause);

#endif // TH_SYSTICK_H


