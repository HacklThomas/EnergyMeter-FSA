//--------------------------------------------------------------
// File     : th_adc.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / RCC / ADC
// Funktion : ADC
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_adc.h"

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void TH_ADC_Init() {
	TH_GPIO_Init(GPIOC, GPIO_PIN_0, TH_GPIO_Mode_AN, TH_GPIO_OType_PP, TH_GPIO_PuPd_DOWN, TH_GPIO_Speed_Medium);
	//TH_GPIO_Init(GPIOC, GPIO_PIN_1, TH_GPIO_Mode_AN, TH_GPIO_OType_PP, TH_GPIO_PuPd_DOWN, TH_GPIO_Speed_Medium);
	/* Init ADC */
	TH_ADC_InitADC(ADC1);
	//TH_ADC_InitADC(ADC2);
}

void TH_ADC_InitADC(ADC_TypeDef* ADCx) {
	ADC_InitTypeDef ADC_InitStruct;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;

	/* Init ADC settings */
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;

	/* Enable clock and fill resolution settings */
	if (ADCx == ADC1) {
		/* Enable ADC clock */
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

		/* Set resolution */
		ADC_InitStruct.ADC_Resolution = TH_ADC1_RESOLUTION;
	} else if (ADCx == ADC2) {
		/* Enable ADC clock */
		RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;

		/* Set resolution */
		ADC_InitStruct.ADC_Resolution = TH_ADC2_RESOLUTION;
	}

	/* Set common ADC settings */
	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_8Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);

	/* Initialize ADC */
	ADC_Init(ADCx, &ADC_InitStruct);

	/* Enable ADC */
	ADCx->CR2 |= ADC_CR2_ADON;
}

uint16_t TH_ADC_Read(ADC_TypeDef* ADCx, uint8_t channel) {
	uint32_t timeout = 0xFFF;

	ADC_RegularChannelConfig(ADCx, channel, 1, ADC_SampleTime_15Cycles);

	/* Start software conversion */
	ADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;

	/* Wait till done */
	while (!(ADCx->SR & ADC_SR_EOC)) {
		if (timeout-- == 0x00) {
			return 0;
		}
	}

	/* Return result */
	return ADCx->DR;
}
