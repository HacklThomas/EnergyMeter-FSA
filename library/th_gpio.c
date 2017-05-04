//--------------------------------------------------------------
// File     : th_gpio.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO
// Funktion : GPIO Funktionen
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_gpio.h"

//--------------------------------------------------------------
// Private Variablen/Funktionen
//--------------------------------------------------------------

static uint16_t GPIO_UsedPins[11] = {0,0,0,0,0,0,0,0,0,0,0};
void TH_GPIO_INT_EnableClock(GPIO_TypeDef* GPIOx);
void TH_GPIO_INT_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TH_GPIO_Mode_t GPIO_Mode, TH_GPIO_OType_t GPIO_OType, TH_GPIO_PuPd_t GPIO_PuPd, TH_GPIO_Speed_t GPIO_Speed);


void TH_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TH_GPIO_Mode_t GPIO_Mode, TH_GPIO_OType_t GPIO_OType, TH_GPIO_PuPd_t GPIO_PuPd, TH_GPIO_Speed_t GPIO_Speed) {
	/* Check input */
	if (GPIO_Pin == 0x00) {
		return;
	}

	/* Enable clock for GPIO */
	TH_GPIO_INT_EnableClock(GPIOx);

	/* Do initialization */
	TH_GPIO_INT_Init(GPIOx, GPIO_Pin, GPIO_Mode, GPIO_OType, GPIO_PuPd, GPIO_Speed);
}

void TH_GPIO_InitAlternate(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TH_GPIO_OType_t GPIO_OType, TH_GPIO_PuPd_t GPIO_PuPd, TH_GPIO_Speed_t GPIO_Speed, uint8_t Alternate) {
	uint32_t pinpos;

	/* Check input */
	if (GPIO_Pin == 0x00) {
		return;
	}

	/* Enable GPIOx clock */
	TH_GPIO_INT_EnableClock(GPIOx);

	/* Set alternate functions for all pins */
	for (pinpos = 0; pinpos < 0x10; pinpos++) {
		/* Check pin */
		if ((GPIO_Pin & (1 << pinpos)) == 0) {
			continue;
		}

		/* Set alternate function */
		GPIOx->AFR[pinpos >> 0x03] = (GPIOx->AFR[pinpos >> 0x03] & ~(0x0F << (4 * (pinpos & 0x07)))) | (Alternate << (4 * (pinpos & 0x07)));
	}

	/* Do initialization */
	TH_GPIO_INT_Init(GPIOx, GPIO_Pin, TH_GPIO_Mode_AF, GPIO_OType, GPIO_PuPd, GPIO_Speed);
}

uint16_t TH_GPIO_GetPortSource(GPIO_TypeDef* GPIOx) {
	/* Get port source number */
	/* Offset from GPIOA                       Difference between 2 GPIO addresses */
	return ((uint32_t)GPIOx - (GPIOA_BASE)) / ((GPIOB_BASE) - (GPIOA_BASE));
}

/* Private functions */
void TH_GPIO_INT_EnableClock(GPIO_TypeDef* GPIOx) {
	/* Set bit according to the 1 << portsourcenumber */
	RCC->AHB1ENR |= (1 << TH_GPIO_GetPortSource(GPIOx));
}

void TH_GPIO_INT_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TH_GPIO_Mode_t GPIO_Mode, TH_GPIO_OType_t GPIO_OType, TH_GPIO_PuPd_t GPIO_PuPd, TH_GPIO_Speed_t GPIO_Speed) {
	uint8_t pinpos;
	uint8_t ptr = TH_GPIO_GetPortSource(GPIOx);

	/* Go through all pins */
	for (pinpos = 0; pinpos < 0x10; pinpos++) {
		/* Check if pin available */
		if ((GPIO_Pin & (1 << pinpos)) == 0) {
			continue;
		}

		/* Pin is used */
		GPIO_UsedPins[ptr] |= 1 << pinpos;

		/* Set GPIO PUPD register */
		GPIOx->PUPDR = (GPIOx->PUPDR & ~(0x03 << (2 * pinpos))) | ((uint32_t)(GPIO_PuPd << (2 * pinpos)));

		/* Set GPIO MODE register */
		GPIOx->MODER = (GPIOx->MODER & ~((uint32_t)(0x03 << (2 * pinpos)))) | ((uint32_t)(GPIO_Mode << (2 * pinpos)));

		/* Set only if output or alternate functions */
		if (GPIO_Mode == TH_GPIO_Mode_OUT || GPIO_Mode == TH_GPIO_Mode_AF) {
			/* Set GPIO OTYPE register */
			GPIOx->OTYPER = (GPIOx->OTYPER & ~(uint16_t)(0x01 << pinpos)) | ((uint16_t)(GPIO_OType << pinpos));

			/* Set GPIO OSPEED register */
			GPIOx->OSPEEDR = (GPIOx->OSPEEDR & ~((uint32_t)(0x03 << (2 * pinpos)))) | ((uint32_t)(GPIO_Speed << (2 * pinpos)));
		}
	}
}

