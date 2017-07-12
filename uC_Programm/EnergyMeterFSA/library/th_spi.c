//--------------------------------------------------------------
// File     : th_spi.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / RCC / SPI
// Funktion : SPI - SD Karte
//--------------------------------------------------------------
//         |PINS PACK 1
//         |MOSI   MISO    SCK
// SPI1    |PA7    PA6     PA5
//--------------------------------------------------------------

#include "th_spi.h"
#include "th_systick.h"

/* Private functions */
static void TH_SPIx_Init(SPI_TypeDef* SPIx, TH_SPI_PinsPack_t pinspack,
		TH_SPI_Mode_t SPI_Mode, uint16_t SPI_BaudRatePrescaler,
		uint16_t SPI_MasterSlave, uint16_t SPI_FirstBit);
void TH_SPI1_INT_InitPins(TH_SPI_PinsPack_t pinspack);

void TH_SPI_Init(SPI_TypeDef* SPIx, TH_SPI_PinsPack_t pinspack) {

	TH_SPIx_Init(SPI1, pinspack, TH_SPI1_MODE, TH_SPI1_PRESCALER,
			TH_SPI1_MASTERSLAVE, TH_SPI1_FIRSTBIT);

}

void TH_SPI_WriteMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint32_t count) {
	uint32_t i;

	/* Check if SPI is enabled */
	SPI_CHECK_ENABLED(SPIx);

	/* Wait for previous transmissions to complete if DMA TX enabled for SPI */
	SPI_WAIT(SPIx);

	for (i = 0; i < count; i++) {
		/* Fill output buffer with data */
		SPIx->DR = dataOut[i];

		/* Wait for SPI to end everything */
		SPI_WAIT(SPIx);

		/* Read data register */
		(void) SPIx->DR;
	}
}

void TH_SPI_ReadMulti(SPI_TypeDef* SPIx, uint8_t* dataIn, uint8_t dummy,
		uint32_t count) {
	uint32_t i;

	/* Check if SPI is enabled */
	SPI_CHECK_ENABLED(SPIx);

	/* Wait for previous transmissions to complete if DMA TX enabled for SPI */
	SPI_WAIT(SPIx);

	for (i = 0; i < count; i++) {
		/* Fill output buffer with data */
		SPIx->DR = dummy;

		/* Wait for SPI to end everything */
		SPI_WAIT(SPIx);

		/* Save data to buffer */
		dataIn[i] = SPIx->DR;
	}
}

/* Private functions */
static void TH_SPIx_Init(SPI_TypeDef* SPIx, TH_SPI_PinsPack_t pinspack,
		TH_SPI_Mode_t SPI_Mode, uint16_t SPI_BaudRatePrescaler,
		uint16_t SPI_MasterSlave, uint16_t SPI_FirstBit) {
	SPI_InitTypeDef SPI_InitStruct;

	/* Set default settings */
	SPI_StructInit(&SPI_InitStruct);
	/* Enable SPI clock */RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	/* Init pins */
	TH_SPI1_INT_InitPins(pinspack);

	/* Set options */
	SPI_InitStruct.SPI_DataSize = TH_SPI1_DATASIZE;

	/* Fill SPI settings */
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit;
	SPI_InitStruct.SPI_Mode = SPI_MasterSlave;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	//SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;

	/* SPI mode */
	if (SPI_Mode == TH_SPI_Mode_0) {
		SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
		SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	} else if (SPI_Mode == TH_SPI_Mode_1) {
		SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
		SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	} else if (SPI_Mode == TH_SPI_Mode_2) {
		SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
		SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	} else if (SPI_Mode == TH_SPI_Mode_3) {
		SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
		SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	}

	/* Disable first */
	SPIx->CR1 &= ~SPI_CR1_SPE;

	/* Init SPI */
	SPI_Init(SPIx, &SPI_InitStruct);

	/* Enable SPI */
	SPIx->CR1 |= SPI_CR1_SPE;
}

void TH_SPI1_INT_InitPins(TH_SPI_PinsPack_t pinspack) {
	if (pinspack == TH_SPI_PinsPack_1) {
		TH_GPIO_InitAlternate(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
				TH_GPIO_OType_PP, TH_GPIO_PuPd_NOPULL, TH_GPIO_Speed_High,
				GPIO_AF_SPI1);
	}
}

