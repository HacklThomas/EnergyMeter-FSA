//--------------------------------------------------------------
// File     : th_led.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / RCC
// Funktion : LED Funktionen
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_led.h"


//--------------------------------------------------------------
// Definition aller LEDs
// Reihenfolge wie bei LED_NAME_t
//
// Init : [LED_OFF,LED_ON]
//--------------------------------------------------------------
LED_t LED[] = {
  // Name    ,PORT , PIN       , CLOCK              , Init
  {LED_GREEN ,GPIOD,GPIO_Pin_12,RCC_AHB1Periph_GPIOD,LED_OFF},   // PD12=Gruene LED
  {LED_RED   ,GPIOD,GPIO_Pin_14,RCC_AHB1Periph_GPIOD,LED_OFF},   // PD14=Rote LED
};



//--------------------------------------------------------------
// Init aller LEDs
//--------------------------------------------------------------
void TH_LED_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  LED_NAME_t led_name;

  for(led_name=0;led_name<LED_ANZ;led_name++) {
    // Clock Enable
    RCC_AHB1PeriphClockCmd(LED[led_name].LED_CLK, ENABLE);

    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = LED[led_name].LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED[led_name].LED_PORT, &GPIO_InitStructure);

    // Default Wert einstellen
    if(LED[led_name].LED_INIT==LED_OFF) {
    	TH_LED_Off(led_name);
    }
    else {
    	TH_LED_On(led_name);
    }
  }
}


//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void TH_LED_Off(LED_NAME_t led_name)
{
  LED[led_name].LED_PORT->BSRRH = LED[led_name].LED_PIN;
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void TH_LED_On(LED_NAME_t led_name)
{
  LED[led_name].LED_PORT->BSRRL = LED[led_name].LED_PIN;
}

//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void TH_LED_Toggle(LED_NAME_t led_name)
{
  LED[led_name].LED_PORT->ODR ^= LED[led_name].LED_PIN;
}
