//--------------------------------------------------------------
// File     : stm32_ub_led.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_LED_H
#define __STM32F4_UB_LED_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

//--------------------------------------------------------------
// Liste aller LEDs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum 
{
  LED_GREEN = 0,  // LED4 auf dem STM32F4-Discovery
  LED_ORANGE = 1, // LED3 auf dem STM32F4-Discovery
  LED_RED = 2,    // LED5 auf dem STM32F4-Discovery
  LED_BLUE = 3    // LED6 auf dem STM32F4-Discovery
}LED_NAME_t;

#define  LED_ANZ   4 // Anzahl von LED_NAME_t


//--------------------------------------------------------------
// Status einer LED
//--------------------------------------------------------------
typedef enum {
  LED_OFF = 0,  // LED AUS
  LED_ON        // LED EIN
}LED_STATUS_t;


//--------------------------------------------------------------
// Struktur einer LED
//--------------------------------------------------------------
typedef struct {
  LED_NAME_t LED_NAME;    // Name
  GPIO_TypeDef* LED_PORT; // Port
  const uint16_t LED_PIN; // Pin
  const uint32_t LED_CLK; // Clock
  LED_STATUS_t LED_INIT;  // Init
}LED_t;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void Led_Init(void);
void Led_Off(LED_NAME_t led_name);
void Led_On(LED_NAME_t led_name);
void Led_Toggle(LED_NAME_t led_name);



//--------------------------------------------------------------
#endif // __STM32F4_UB_LED_H
