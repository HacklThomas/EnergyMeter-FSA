//--------------------------------------------------------------
// File     : th_systick.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : RCC
// Funktion : Timer Funktionen
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_systick.h"
#include "main.h"

//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
static volatile uint32_t Systick_Delay; // Globaler Pausenzaehler

__IO uint32_t TH_Time2 = 0;
static uint32_t temporary;

//--------------------------------------------------------------
// Init vom System-Counter
// entweder im 1us-Takt oder 1ms-Takt
//--------------------------------------------------------------
void TH_SYSTICK_Init(void) {
	RCC_ClocksTypeDef RCC_Clocks;

	// alle Variabeln zurücksetzen
	Systick_Delay = 0;
	TH_Time2 = 0;
	temporary = 0;

	// Timer auf 1us einstellen
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000000);
}

//--------------------------------------------------------------
// Pausenfunktion (in us)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void TH_SYSTICK_Pause_us(volatile uint32_t pause) {
	Systick_Delay = pause;

	while (Systick_Delay != 0)
		;
}

//--------------------------------------------------------------
// Pausenfunktion (in ms)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void TH_SYSTICK_Pause_ms(volatile uint32_t pause) {
	uint32_t ms;

	for (ms = 0; ms < pause; ms++) {
		TH_SYSTICK_Pause_us(1000);
	}
}

//--------------------------------------------------------------
// Pausenfunktion (in s)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void TH_SYSTICK_Pause_s(volatile uint32_t pause) {
	uint32_t s;

	for (s = 0; s < pause; s++) {
		TH_SYSTICK_Pause_ms(1000);
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
		if (TH_Time2 != 0x00) {
			TH_Time2--;
		}
	} else {
		temporary++;
	}
	timer++;
	blink++;
	interrupttimer++;
}

