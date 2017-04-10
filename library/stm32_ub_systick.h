//--------------------------------------------------------------
// File     : stm32_ub_systick.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SYSTICK_H
#define __STM32F4_UB_SYSTICK_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"

extern __IO uint32_t TM_Time2;
//--------------------------------------------------------------
// Auflösung der Systick
// (entweder 1us oder 1000us als Auflösung einstellen)
//--------------------------------------------------------------
#define  SYSTICK_RESOLUTION   1    // 1us Auflösung
//#define  SYSTICK_RESOLUTION   1000   // 1ms Auflösung

#define TM_DELAY_Time2()				(TM_Time2)
#define TM_DELAY_SetTime2(time)			(TM_Time2 = (time))

//--------------------------------------------------------------
// Globale Pausen-Funktionen
//--------------------------------------------------------------
void UB_Systick_Init(void);
#if SYSTICK_RESOLUTION==1
  void UB_Systick_Pause_us(volatile uint32_t pause);
#endif
void UB_Systick_Pause_ms(volatile uint32_t pause);
void UB_Systick_Pause_s(volatile uint32_t pause);

//--------------------------------------------------------------
#endif // __STM32F4_UB_SYSTICK_H


