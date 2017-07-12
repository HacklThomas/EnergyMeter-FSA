//--------------------------------------------------------------
// File     : th_crc.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : RCC / CRC
// Funktion : Cyclic Redundancy Check
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_crc.h"

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void TH_CRC_Init(void) {
	/* Enable CRC clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
}

void TH_CRC_DeInit(void) {
	/* Disable CRC clock */
	RCC->AHB1ENR &= ~RCC_AHB1ENR_CRCEN;
}

uint32_t TH_CRC_Calculate8(uint8_t* arr, uint32_t count, uint8_t reset) {
	/* Reset CRC data register if necessary */
	if (reset) {
		/* Reset generator */
		CRC->CR = CRC_CR_RESET;
	}

	/* Calculate CRC */
	while (count--) {
		/* Set new value */
		CRC->DR = *arr++;
	}

	/* Return data */
	return CRC->DR;
}

uint32_t TH_CRC_Calculate16(uint16_t* arr, uint32_t count, uint8_t reset) {
	/* Reset CRC data register if necessary */
	if (reset) {
		/* Reset generator */
		CRC->CR = CRC_CR_RESET;
	}

	/* Calculate CRC */
	while (count--) {
		/* Set new value */
		CRC->DR = *arr++;
	}

	/* Return data */
	return CRC->DR;
}

uint32_t TH_CRC_Calculate32(uint32_t* arr, uint32_t count, uint8_t reset) {
	/* Reset CRC data register if necessary */
	if (reset) {
		/* Reset generator */
		CRC->CR = CRC_CR_RESET;
	}

	/* Calculate CRC */
	while (count--) {
		/* Set new value */
		CRC->DR = *arr++;
	}

	/* Return data */
	return CRC->DR;
}
