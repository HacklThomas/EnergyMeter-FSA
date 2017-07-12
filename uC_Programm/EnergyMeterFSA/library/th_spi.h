//--------------------------------------------------------------
// File     : th_spi.h
//--------------------------------------------------------------

#ifndef TH_SPI_H
#define TH_SPI_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "defines.h"
#include "th_gpio.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------
typedef enum {
	TH_SPI_Mode_0, /*!< Clock polarity low, clock phase 1st edge */
	TH_SPI_Mode_1, /*!< Clock polarity low, clock phase 2nd edge */
	TH_SPI_Mode_2, /*!< Clock polarity high, clock phase 1st edge */
	TH_SPI_Mode_3  /*!< Clock polarity high, clock phase 2nd edge */
} TH_SPI_Mode_t;

typedef enum {
	TH_SPI_PinsPack_1,       /*!< Select PinsPack1 from Pinout table for specific SPI */
	TH_SPI_PinsPack_2,       /*!< Select PinsPack2 from Pinout table for specific SPI */
	TH_SPI_PinsPack_3,       /*!< Select PinsPack3 from Pinout table for specific SPI */
	TH_SPI_PinsPack_Custom   /*!< Select custom pins for specific SPI, callback will be called, look @ref TM_SPI_InitCustomPinsCallback */
} TH_SPI_PinsPack_t;

typedef enum {
	TH_SPI_DataSize_8b, /*!< SPI in 8-bits mode */
	TH_SPI_DataSize_16b /*!< SPI in 16-bits mode */
} TH_SPI_DataSize_t;

//----- SPI1 options start -------
//Options can be overwriten in defines.h file
#ifndef TH_SPI1_PRESCALER
#define TH_SPI1_PRESCALER	SPI_BaudRatePrescaler_32
#endif
//Specify datasize
#ifndef TH_SPI1_DATASIZE
#define TH_SPI1_DATASIZE 	SPI_DataSize_8b
#endif
//Specify which bit is first
#ifndef TH_SPI1_FIRSTBIT
#define TH_SPI1_FIRSTBIT 	SPI_FirstBit_MSB
#endif
//Mode, master or slave
#ifndef TH_SPI1_MASTERSLAVE
#define TH_SPI1_MASTERSLAVE	SPI_Mode_Master
#endif
//Specify mode of operation, clock polarity and clock phase
#ifndef TH_SPI1_MODE
#define TH_SPI1_MODE		TH_SPI_Mode_0
#endif
//----- SPI1 options end -------

#define SPI_IS_BUSY(SPIx) (((SPIx)->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 || ((SPIx)->SR & SPI_SR_BSY))

#define SPI_WAIT(SPIx)            while (SPI_IS_BUSY(SPIx))

#define SPI_CHECK_ENABLED(SPIx)   if (!((SPIx)->CR1 & SPI_CR1_SPE)) {return;}

#define SPI_CHECK_ENABLED_RESP(SPIx, val)   if (!((SPIx)->CR1 & SPI_CR1_SPE)) {return (val);}

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
/**
 * @brief  Initializes SPIx peripheral with custom pinspack and default other settings
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  pinspack: Pinspack you will use from default SPI table. This parameter can be a value of @ref TM_SPI_PinsPack_t enumeration
 * @retval None
 */
void TH_SPI_Init(SPI_TypeDef* SPIx, TH_SPI_PinsPack_t pinspack);

/**
 * @brief  Sends single byte over SPI
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  data: 8-bit data size to send over SPI
 * @retval Received byte from slave device
 */
static __INLINE uint8_t TH_SPI_Send(SPI_TypeDef* SPIx, uint8_t data) {
	/* Check if SPI is enabled */
	SPI_CHECK_ENABLED_RESP(SPIx, 0);

	/* Wait for previous transmissions to complete if DMA TX enabled for SPI */
	SPI_WAIT(SPIx);

	/* Fill output buffer with data */
	SPIx->DR = data;

	/* Wait for transmission to complete */
	SPI_WAIT(SPIx);

	/* Return data from buffer */
	return SPIx->DR;
}

/**
 * @brief  Writes multiple bytes over SPI
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataOut: Pointer to array with data to send over SPI
 * @param  count: Number of elements to send over SPI
 * @retval None
 */
void TH_SPI_WriteMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint32_t count);

/**
 * @brief  Receives multiple data bytes over SPI
 * @note   Selected SPI must be set in 16-bit mode
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataIn: Pointer to 8-bit array to save data into
 * @param  dummy: Dummy byte  to be sent over SPI, to receive data back. In most cases 0x00 or 0xFF
 * @param  count: Number of bytes you want read from device
 * @retval None
 */
void TH_SPI_ReadMulti(SPI_TypeDef* SPIx, uint8_t *dataIn, uint8_t dummy, uint32_t count);

#endif // TH_SPI_H

