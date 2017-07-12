//--------------------------------------------------------------
// File     : th_id.h
//--------------------------------------------------------------
#ifndef TH_IDENTIFICATION_H
#define TH_IDENTIFICATION_H 100

#include "stm32f4xx.h"

// Unique ID register address location
#define ID_UNIQUE_ADDRESS		0x1FFF7A10
// brief Flash size register address
#define ID_FLASH_ADDRESS		0x1FFF7A22
// brief Device ID register address
#define ID_DBGMCU_IDCODE		0xE0042000

// Device signature, bits 11:0 are valid, 15:12 are always 0.
#define TH_ID_GetSignature()	((*(uint16_t *) (ID_DBGMCU_IDCODE)) & 0x0FFF)
// Device revision value
#define TH_ID_GetRevision()		(*(uint16_t *) (DBGMCU->IDCODE + 2))
//Flash size in kilo bytes
#define TH_ID_GetFlashSize()	(*(uint16_t *) (ID_FLASH_ADDRESS))

// STM32F4xx has 96bits long unique ID, so 12 bytes are available for read in 8-bit format
#define TH_ID_GetUnique8(x)		((x >= 0 && x < 12) ? (*(uint8_t *) (ID_UNIQUE_ADDRESS + (x))) : 0)
// STM32F4xx has 96bits long unique ID, so 6 2-bytes values are available for read in 16-bit format
#define TH_ID_GetUnique16(x)	((x >= 0 && x < 6) ? (*(uint16_t *) (ID_UNIQUE_ADDRESS + 2 * (x))) : 0)
// STM32F4xx has 96bits long unique ID, so 3 4-bytes values are available for read in 32-bit format
#define TH_ID_GetUnique32(x)	((x >= 0 && x < 3) ? (*(uint32_t *) (ID_UNIQUE_ADDRESS + 4 * (x))) : 0)

#endif // TH_IDENTIFICATION_H
