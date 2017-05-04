//--------------------------------------------------------------
// File     : th_gpio.h
//--------------------------------------------------------------
#ifndef TH_GPIO_H
#define TH_GPIO_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "defines.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------

#ifndef GPIO_PIN_0
#define GPIO_PIN_0		((uint16_t)0x0001)
#define GPIO_PIN_1		((uint16_t)0x0002)
#define GPIO_PIN_2		((uint16_t)0x0004)
#define GPIO_PIN_3		((uint16_t)0x0008)
#define GPIO_PIN_4		((uint16_t)0x0010)
#define GPIO_PIN_5		((uint16_t)0x0020)
#define GPIO_PIN_6		((uint16_t)0x0040)
#define GPIO_PIN_7		((uint16_t)0x0080)
#define GPIO_PIN_8		((uint16_t)0x0100)
#define GPIO_PIN_9		((uint16_t)0x0200)
#define GPIO_PIN_10		((uint16_t)0x0400)
#define GPIO_PIN_11		((uint16_t)0x0800)
#define GPIO_PIN_12		((uint16_t)0x1000)
#define GPIO_PIN_13		((uint16_t)0x2000)
#define GPIO_PIN_14		((uint16_t)0x4000)
#define GPIO_PIN_15		((uint16_t)0x8000)
#define GPIO_PIN_ALL	((uint16_t)0xFFFF)
#endif

typedef enum {
	TH_GPIO_Mode_IN = 0x00,  /*!< GPIO Pin as General Purpose Input */
	TH_GPIO_Mode_OUT = 0x01, /*!< GPIO Pin as General Purpose Output */
	TH_GPIO_Mode_AF = 0x02,  /*!< GPIO Pin as Alternate Function */
	TH_GPIO_Mode_AN = 0x03,  /*!< GPIO Pin as Analog */
} TH_GPIO_Mode_t;

typedef enum {
	TH_GPIO_OType_PP = 0x00, /*!< GPIO Output Type Push-Pull */
	TH_GPIO_OType_OD = 0x01  /*!< GPIO Output Type Open-Drain */
} TH_GPIO_OType_t;

typedef enum {
	TH_GPIO_Speed_Low = 0x00,    /*!< GPIO Speed Low */
	TH_GPIO_Speed_Medium = 0x01, /*!< GPIO Speed Medium */
	TH_GPIO_Speed_Fast = 0x02,   /*!< GPIO Speed Fast */
	TH_GPIO_Speed_High = 0x03    /*!< GPIO Speed High */
} TH_GPIO_Speed_t;

typedef enum {
	TH_GPIO_PuPd_NOPULL = 0x00, /*!< No pull resistor */
	TH_GPIO_PuPd_UP = 0x01,     /*!< Pull up resistor enabled */
	TH_GPIO_PuPd_DOWN = 0x02    /*!< Pull down resistor enabled */
} TH_GPIO_PuPd_t;

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
/**
 * @brief  Initializes GPIO pins(s)
 * @note   This function also enables clock for GPIO port
 * @param  GPIOx: Pointer to GPIOx port you will use for initialization
 * @param  GPIO_Pin: GPIO pin(s) you will use for initialization
 * @param  GPIO_Mode: Select GPIO mode. This parameter can be a value of @ref TM_GPIO_Mode_t enumeration
 * @param  GPIO_OType: Select GPIO Output type. This parameter can be a value of @ref TM_GPIO_OType_t enumeration
 * @param  GPIO_PuPd: Select GPIO pull resistor. This parameter can be a value of @ref TM_GPIO_PuPd_t enumeration
 * @param  GPIO_Speed: Select GPIO speed. This parameter can be a value of @ref TM_GPIO_Speed_t enumeration
 * @retval None
 */
void TH_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TH_GPIO_Mode_t GPIO_Mode, TH_GPIO_OType_t GPIO_OType, TH_GPIO_PuPd_t GPIO_PuPd, TH_GPIO_Speed_t GPIO_Speed);

/**
 * @brief  Initializes GPIO pins(s) as alternate function
 * @note   This function also enables clock for GPIO port
 * @param  GPIOx: Pointer to GPIOx port you will use for initialization
 * @param  GPIO_Pin: GPIO pin(s) you will use for initialization
 * @param  GPIO_OType: Select GPIO Output type. This parameter can be a value of @ref TM_GPIO_OType_t enumeration
 * @param  GPIO_PuPd: Select GPIO pull resistor. This parameter can be a value of @ref TM_GPIO_PuPd_t enumeration
 * @param  GPIO_Speed: Select GPIO speed. This parameter can be a value of @ref TM_GPIO_Speed_t enumeration
 * @param  Alternate: Alternate function you will use
 * @retval None
 */
void TH_GPIO_InitAlternate(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TH_GPIO_OType_t GPIO_OType, TH_GPIO_PuPd_t GPIO_PuPd, TH_GPIO_Speed_t GPIO_Speed, uint8_t Alternate);


#define TH_GPIO_SetPinHigh(GPIOx, GPIO_Pin) 		((GPIOx)->BSRRL = (GPIO_Pin))

/**
 * @brief  Gets input data bit
 * @note   Defined as macro to get maximum speed using register access
 * @param  GPIOx: GPIOx PORT where you want to read input bit value
 * @param  GPIO_Pin: GPIO pin where you want to read value
 * @retval 1 in case pin is high, or 0 if low
 */
#define TH_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)

/**
 * @brief  Gets port source from desired GPIOx PORT
 * @note   Meant for private use, unless you know what are you doing
 * @param  GPIOx: GPIO PORT for calculating port source
 * @retval Calculated port source for GPIO
 */
uint16_t TH_GPIO_GetPortSource(GPIO_TypeDef* GPIOx);

#endif
