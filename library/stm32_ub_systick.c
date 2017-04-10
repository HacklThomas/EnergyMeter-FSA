//--------------------------------------------------------------
// File     : stm32_ub_systick.c
// Datum    : 13.03.2013
// Version  : 1.4
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : keine
// Funktion : Pausen- Timer- und Counter-Funktionen
//            Zeiten im [us,ms,s] Raster
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_systick.h"
#include "main.h"

#if ((SYSTICK_RESOLUTION!=1) && (SYSTICK_RESOLUTION!=1000))
#error print WRONG SYSTICK RESOLUTION !
#endif

//--------------------------------------------------------------
// Globale Pausen-Variabeln
//--------------------------------------------------------------
static volatile uint32_t Systick_Delay; // Globaler Pausenzaehler

__IO uint32_t TM_Time2 = 0;
static volatile uint32_t temporary;

//--------------------------------------------------------------
// Init vom System-Counter
// entweder im 1us-Takt oder 1ms-Takt
//--------------------------------------------------------------
void UB_Systick_Init(void) {
	RCC_ClocksTypeDef RCC_Clocks;

	// alle Variabeln zurücksetzen
	Systick_Delay = 0;
	TM_Time2 = 0;
	temporary = 0;

#if SYSTICK_RESOLUTION==1
	// Timer auf 1us einstellen
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000000);
#else
	// Timer auf 1ms einstellen
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
#endif
}

#if SYSTICK_RESOLUTION==1
//--------------------------------------------------------------
// Pausenfunktion (in us)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void UB_Systick_Pause_us(volatile uint32_t pause) {

	Systick_Delay = pause;

	while (Systick_Delay != 0)
		;
}
#endif

//--------------------------------------------------------------
// Pausenfunktion (in ms)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void UB_Systick_Pause_ms(volatile uint32_t pause) {
#if SYSTICK_RESOLUTION==1
	uint32_t ms;

	for (ms = 0; ms < pause; ms++) {
		UB_Systick_Pause_us(1000);
	}
#else
	Systick_Delay = pause;

	while(Systick_Delay != 0);
#endif
}

//--------------------------------------------------------------
// Pausenfunktion (in s)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void UB_Systick_Pause_s(volatile uint32_t pause) {
	uint32_t s;

	for (s = 0; s < pause; s++) {
		UB_Systick_Pause_ms(1000);
	}
}

//--------------------------------------------------------------
// Systic-Interrupt
//--------------------------------------------------------------
void SysTick_Handler(void) {
	// Tick für Pause
	if (Systick_Delay != 0x00) {
		Systick_Delay--;
	}

	if (temporary >= 1000) {
		temporary = 0;
		if (TM_Time2 != 0x00) {
			TM_Time2--;
		}
	} else {
		temporary++;
	}
	timer++;
	interrupttimer++;
	savedatatimer++;
	lastcantimer++;
}

