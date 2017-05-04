//--------------------------------------------------------------
// File     : th_adc.h
//-------------------------------------------------------------

#ifndef TH_ADC_H
#define TH_ADC_H 120

/*
 * ADC1 / CHANNEL10 : PC0 - LV
 * ADC2 / CHANNEL11 : PC1 - LV current
 */

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "defines.h"
#include "th_gpio.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------
#ifndef TH_ADC1_RESOLUTION
#define TH_ADC1_RESOLUTION		ADC_Resolution_10b
#endif

#ifndef TH_ADC2_RESOLUTION
#define TH_ADC2_RESOLUTION		ADC_Resolution_12b
#endif

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------

/**
 * @brief  Initializes ADCx peripheral
 * @param  *ADCx: ADCx peripheral to initialize
 * @retval None
 */
void TH_ADC_InitADC(ADC_TypeDef* ADCx);

/**
 * @brief  Initializes ADCx with ADCx channel
 * @param  *ADCx: ADCx peripheral to operate with
 * @param  channel: channel for ADCx
 * @retval None
 */
void TH_ADC_Init();

/**
 * @brief  Reads from ADCx channel
 * @param  *ADCx: ADCx peripheral to operate with
 * @param  channel: channel for ADCx to read from
 * @retval ADC value
 */
uint16_t TH_ADC_Read(ADC_TypeDef* ADCx, uint8_t channel);


#endif
