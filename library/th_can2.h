//--------------------------------------------------------------
// File     : th_can2.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef TH_CAN2_H
#define TH_CAN2_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_can.h"
#include "misc.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------

//--------------------------------------------------------------
//   Prescaler : 2 ->   1 MBit/sec  [FRQ = 1MHz]
//               4 -> 500 kBit/sec  [FRQ = 500 kHz]
//               8 -> 250 kBit/sec  [FRQ = 250 kHz]
//              16 -> 125 kBit/sec  [FRQ = 125 kHz]
//              20 -> 100 kBit/sec  [FRQ = 100 kHz]
//              40 ->  50 kBit/sec  [FRQ = 50 kHz]
//             100 ->  20 kBit/sec  [FRQ = 20 kHz]
//             200 ->  10 kBit/sec  [FRQ = 10 kHz]
//--------------------------------------------------------------
#define   CAN2_CLOCK_PRESCALER    4

typedef struct {
  GPIO_TypeDef* PORT;     // Port
  const uint16_t PIN;     // Pin
  const uint32_t CLK;     // Clock
  const uint8_t SOURCE;   // Source
}CAN2_PIN_t;

typedef struct {
  CAN2_PIN_t  TX;       // TX-Pin
  CAN2_PIN_t  RX;       // RX-Pin
}CAN2_DEV_t;

typedef struct {
  uint32_t can_id;    // STD=11bit  EXT=29bit
  uint8_t anz_bytes;  // anzahl der daten [0...8]
  uint8_t data[8];    // datenbytes
}CAN2_TX_FRAME_t;

typedef enum {
  CAN2_STD_DATA =0, // standard Daten-Frame
  CAN2_STD_REMOTE,  // standard Remote-Frame
  CAN2_EXT_DATA,    // extended Daten-Frame
  CAN2_EXT_REMOTE   // extended Remote-Frame
}CAN2_FRAME_MODE_t;

typedef struct {
  CAN2_FRAME_MODE_t frame_mode;    // Frame-Mode
  uint32_t can_id;    // STD=11bit  EXT=29bit
  uint8_t anz_bytes;  // anzahl der daten [0...8]
  uint8_t data[8];    // datenbytes
}CAN2_RX_FRAME_t;

typedef enum {
  CAN2_RX_EMPTY =0,  // noch nichts empfangen
  CAN2_RX_READY      // etwas wurde empfangen
}CAN2_STATUS_t;

typedef struct {
  CAN2_STATUS_t status; // RX-Status
  CAN2_FRAME_MODE_t frame_mode;      // Frame-Mode
  uint32_t can_id;      // STD=11bit  EXT=29bit
  uint8_t anz_bytes;    // anzahl der daten [0...8]
  uint8_t data[8];      // datenbytes
}CAN2_BUFFER_t;


//-------------------------------------
#define  CAN2_TX_TIMEOUT  500000  // > 5000

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void TH_CAN2_Init(void);
ErrorStatus TH_CAN2_send_std_data(CAN2_TX_FRAME_t tx_frame);
CAN2_STATUS_t TH_CAN2_receive(CAN2_RX_FRAME_t *rx_frame);

//--------------------------------------------------------------
#endif // TH_CAN2_H
