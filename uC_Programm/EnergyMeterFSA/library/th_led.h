//--------------------------------------------------------------
// File     : th_led.h
//--------------------------------------------------------------

#ifndef TH_LED_H
#define TH_LED_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------

#define  LED_ANZ   2 // Anzahl von LED_NAME_t

typedef enum
{
  LED_GREEN = 0,
  LED_RED = 1
}LED_NAME_t;

typedef enum {
  LED_OFF = 0,  // LED AUS
  LED_ON        // LED EIN
}LED_STATUS_t;

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

void TH_LED_Init(void);
void TH_LED_Off(LED_NAME_t led_name);
void TH_LED_On(LED_NAME_t led_name);
void TH_LED_Toggle(LED_NAME_t led_name);

#endif // TH_LED_H
