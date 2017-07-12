//--------------------------------------------------------------
// File     : th_adc.h
//-------------------------------------------------------------
#ifndef TH_CRC_H
#define TH_CRC_H 110

/*
 * Library uses hardware CRC unit in STM32F4xx device
*/

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "defines.h"

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------

/**
 * @brief  Initializes and enables CRC peripheral
 * @param  None
 * @retval None
 */
void TH_CRC_Init(void);

/**
 * @brief  De initializes and disable CRC peripheral
 * @param  None
 * @retval None
 */
void TH_CRC_DeInit(void);

/**
 * @brief  Calculates 32-bit CRC value from 8-bit input array
 * @param  *arr: Pointer to 8-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @param  reset: Reset CRC peripheral to 0 state before starting new calculations
 *            - 0: CRC unit will not be reset before new calculations will happen and will use previous data to continue
 *            - > 0: CRC unit is set to 0 before first calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t TH_CRC_Calculate8(uint8_t* arr, uint32_t count, uint8_t reset);

/**
 * @brief  Calculates 32-bit CRC value from 16-bit input array
 * @param  *arr: Pointer to 16-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @param  reset: Reset CRC peripheral to 0 state before starting new calculations
 *            - 0: CRC unit will not be reset before new calculations will happen and will use previous data to continue
 *            - > 0: CRC unit is set to 0 before first calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t TH_CRC_Calculate16(uint16_t* arr, uint32_t count, uint8_t reset);

/**
 * @brief  Calculates 32-bit CRC value from 32-bit input array
 * @param  *arr: Pointer to 32-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @param  reset: Reset CRC peripheral to 0 state before starting new calculations
 *            - 0: CRC unit will not be reset before new calculations will happen and will use previous data to continue
 *            - > 0: CRC unit is set to 0 before first calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t TH_CRC_Calculate32(uint32_t* arr, uint32_t count, uint8_t reset);

#endif
