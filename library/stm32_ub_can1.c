//--------------------------------------------------------------
// File     : stm32_ub_can1.c
// Datum    : 21.09.2013
// Version  : 1.2
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, CAN, MISC
// Funktion : CAN-LoLevel-Funktionen (per CAN-1)
//            externer CAN-Transeiver wird benötigt
//
// Hinweis  : mögliche Pinbelegungen
//            CAN1 : TX:[PA12, PB9, PD1]
//                   RX:[PA11, PB8, PD0]
//
// Terminations-Widerstand (120 Ohm) am CAN-Bus nicht vergessen !
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_can1.h"


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P1_init_NVIC(void);
void P1_init_GPIO(void);
void P1_init_CAN(void);
void P1_receive(void);
uint16_t P1_make_stdFL(CAN1_STD_FL_t filter, uint8_t nr);
uint16_t P1_make_stdFM(CAN1_STD_FM_t filter, uint8_t nr);
uint16_t P1_make_extFL(CAN1_EXT_FL_t filter, uint8_t nr);
uint16_t P1_make_extFM(CAN1_EXT_FM_t filter, uint8_t nr);


//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
CanRxMsg CAN1_RxMessage;
CanTxMsg CAN1_TxMessage;
CAN1_BUFFER_t can1_buffer;


//--------------------------------------------------------------
// Definition von CAN1
//--------------------------------------------------------------
CAN1_DEV_t CAN1DEV = {
// PORT , PIN      , Clock              , Source
  {GPIOB,GPIO_Pin_9,RCC_AHB1Periph_GPIOB,GPIO_PinSource9}, // TX an PB9
  {GPIOB,GPIO_Pin_8,RCC_AHB1Periph_GPIOB,GPIO_PinSource8}, // RX an PB8
};



//--------------------------------------------------------------
// Init von CAN1
//--------------------------------------------------------------
void UB_CAN1_Init(void)
{
  static uint8_t init_ok=0;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    return;
  }

  // init vom NVIC
  P1_init_NVIC();

  // init der GPIO
  P1_init_GPIO();

  // init vom CAN
  P1_init_CAN();

  // init Mode speichern
  init_ok=1;
}


//--------------------------------------------------------------
// standard Data-Frame senden
// tx_frame :
// can_id    = Message Identifier [0...0x7FF]
// anz_bytes = Anzahl der Daten die gesendet werden
// data[]    = Daten die gesendet werden
//
// Return_wert :
//  -> ERROR   , Frame wurde nicht gesendet
//  -> SUCCESS , Frame wurde gesendet
//--------------------------------------------------------------
ErrorStatus UB_CAN1_send_std_data(CAN1_TX_FRAME_t tx_frame)
{
  uint8_t n;
  uint32_t timeout;

  if(tx_frame.can_id>0x7FF) {
    return(ERROR);
  }

  if(tx_frame.anz_bytes>8) {
    return(ERROR);
  }

  // daten frame vorbereiten
  CAN1_TxMessage.StdId = tx_frame.can_id;
  CAN1_TxMessage.ExtId = 0x00;
  CAN1_TxMessage.RTR = CAN_RTR_DATA;
  CAN1_TxMessage.IDE = CAN_ID_STD;
  CAN1_TxMessage.DLC = tx_frame.anz_bytes;

  for(n=0;n<tx_frame.anz_bytes;n++) {
    CAN1_TxMessage.Data[n] = tx_frame.data[n];
  }

  // warten bis ein Transmit Register frei ist
  timeout=0;
  while(!(CAN1->TSR & CAN_TSR_TME0 || CAN1->TSR & CAN_TSR_TME1 || CAN1->TSR & CAN_TSR_TME2)) {
    timeout++;
    if(timeout>CAN1_TX_TIMEOUT)  break;
  }

  // daten frame senden
  CAN_Transmit(CAN1, &CAN1_TxMessage);

  return(SUCCESS);
}


//--------------------------------------------------------------
// standard Remote-Frame senden
// tx_frame :
// can_id    = Message Identifier [0...0x7FF]
// anz_bytes = Anzahl der Daten die erwartet werden
// data[]    = nicht notwendig
//
// Return_wert :
//  -> ERROR   , Frame wurde nicht gesendet
//  -> SUCCESS , Frame wurde gesendet
//--------------------------------------------------------------
ErrorStatus UB_CAN1_send_std_remote(CAN1_TX_FRAME_t tx_frame)
{
  uint32_t timeout;

  if(tx_frame.can_id>0x7FF) {
    return(ERROR);
  }

  if(tx_frame.anz_bytes>8) {
    return(ERROR);
  }

  // remote frame vorbereiten
  CAN1_TxMessage.StdId = tx_frame.can_id;
  CAN1_TxMessage.ExtId = 0x00;
  CAN1_TxMessage.RTR = CAN_RTR_REMOTE;
  CAN1_TxMessage.IDE = CAN_ID_STD;
  CAN1_TxMessage.DLC = tx_frame.anz_bytes;

  // warten bis ein Transmit Register frei ist
  timeout=0;
  while(!(CAN1->TSR & CAN_TSR_TME0 || CAN1->TSR & CAN_TSR_TME1 || CAN1->TSR & CAN_TSR_TME2)) {
    timeout++;
    if(timeout>CAN1_TX_TIMEOUT)  break;
  }

  // remote frame senden
  CAN_Transmit(CAN1, &CAN1_TxMessage);

  return(SUCCESS);
}


//--------------------------------------------------------------
// extended Data-Frame senden
// tx_frame :
// can_id    = Message Identifier [0...0x1FFFFFFF]
// anz_bytes = Anzahl der Daten die gesendet werden
// data[]    = Daten die gesendet werden
//
// Return_wert :
//  -> ERROR   , Frame wurde nicht gesendet
//  -> SUCCESS , Frame wurde gesendet
//--------------------------------------------------------------
ErrorStatus UB_CAN1_send_ext_data(CAN1_TX_FRAME_t tx_frame)
{
  uint8_t n;
  uint32_t timeout;

  if(tx_frame.can_id>0x1FFFFFFF) {
    return(ERROR);
  }

  if(tx_frame.anz_bytes>8) {
    return(ERROR);
  }

  // daten frame vorbereiten
  CAN1_TxMessage.StdId = 0x00;
  CAN1_TxMessage.ExtId = tx_frame.can_id;
  CAN1_TxMessage.RTR = CAN_RTR_DATA;
  CAN1_TxMessage.IDE = CAN_ID_EXT;
  CAN1_TxMessage.DLC = tx_frame.anz_bytes;

  for(n=0;n<tx_frame.anz_bytes;n++) {
    CAN1_TxMessage.Data[n] = tx_frame.data[n];
  }

  // warten bis ein Transmit Register frei ist
  timeout=0;
  while(!(CAN1->TSR & CAN_TSR_TME0 || CAN1->TSR & CAN_TSR_TME1 || CAN1->TSR & CAN_TSR_TME2)) {
    timeout++;
    if(timeout>CAN1_TX_TIMEOUT)  break;
  }

  // daten frame senden
  CAN_Transmit(CAN1, &CAN1_TxMessage);

  return(SUCCESS);
}


//--------------------------------------------------------------
// extended Remote-Frame senden
// tx_frame :
// can_id    = Message Identifier [0...0x1FFFFFFF]
// anz_bytes = Anzahl der Daten die erwartet werden
// data[]    = nicht notwendig
//
// Return_wert :
//  -> ERROR   , Frame wurde nicht gesendet
//  -> SUCCESS , Frame wurde gesendet
//--------------------------------------------------------------
ErrorStatus UB_CAN1_send_ext_remote(CAN1_TX_FRAME_t tx_frame)
{
  uint32_t timeout;

  if(tx_frame.can_id>0x1FFFFFFF) {
    return(ERROR);
  }

  if(tx_frame.anz_bytes>8) {
    return(ERROR);
  }

  // remote frame vorbereiten
  CAN1_TxMessage.StdId = 0x00;
  CAN1_TxMessage.ExtId = tx_frame.can_id;
  CAN1_TxMessage.RTR = CAN_RTR_REMOTE;
  CAN1_TxMessage.IDE = CAN_ID_EXT;
  CAN1_TxMessage.DLC = tx_frame.anz_bytes;

  // warten bis ein Transmit Register frei ist
  timeout=0;
  while(!(CAN1->TSR & CAN_TSR_TME0 || CAN1->TSR & CAN_TSR_TME1 || CAN1->TSR & CAN_TSR_TME2)) {
    timeout++;
    if(timeout>CAN1_TX_TIMEOUT)  break;
  }

  // remote frame senden
  CAN_Transmit(CAN1, &CAN1_TxMessage);

  return(SUCCESS);
}


//--------------------------------------------------------------
// check ob CAN-Frame empfangen wurde
// return_wert :
//    CAN1_RX_EMPTY = nichts empfangen
//    CAN1_RX_READY = CAN-Frame empfangen
//
// rx_frame :
// frame_mode = [CAN1_STD_DATA,CAN1_STD_REMOTE,CAN1_EXT_DATA,CAN1_EXT_REMOTE]
// can_id     = Message Identifier
// anz_bytes  = Anzahl der Daten
// data[]     = Daten die empfangen wurden (nur bei Data-Frame)
//--------------------------------------------------------------
CAN1_STATUS_t UB_CAN1_receive(CAN1_RX_FRAME_t *rx_frame)
{
  CAN1_STATUS_t ret_wert=CAN1_RX_EMPTY;
  uint8_t n;

  if(can1_buffer.status==CAN1_RX_READY) {
    // wenn Frame empfangen wurde -> inhalt kopieren
    rx_frame->frame_mode=can1_buffer.frame_mode;
    rx_frame->can_id=can1_buffer.can_id;
    rx_frame->anz_bytes=can1_buffer.anz_bytes;
    for(n=0;n<8;n++) {
      rx_frame->data[n]=can1_buffer.data[n];
    }

    ret_wert=CAN1_RX_READY;
    can1_buffer.status=CAN1_RX_EMPTY;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// setzt vier Filter auf std_id [11bit] + Mode
// falls ein Filter nicht benötigt wird,
// dann den Filter doppelt anlegen
//
// Ein CAN-Frame kommt nur durch, wenn seine ID+Mode
// in der Liste steht
//
// nr = [0...13] (für CAN1)
//--------------------------------------------------------------
void UB_CAN1_std_FilterList(CAN1_STD_FL_t filter, uint8_t nr)
{
  if(nr>13) nr=0;

  // CAN Filter
  CAN_FilterInitStructure.CAN_FilterNumber = nr;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = P1_make_stdFL(filter,2);     // ID2
  CAN_FilterInitStructure.CAN_FilterIdLow = P1_make_stdFL(filter,1);      // ID1
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P1_make_stdFL(filter,4); // ID4
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = P1_make_stdFL(filter,3);  // ID3
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}


//--------------------------------------------------------------
// setzt zwei Filter auf std_id [11bit] + Mode mit Maske
// falls ein Filter nicht benötigt wird,
// dann den Filter doppelt anlegen
//
// Ein CAN-Frame kommt nur durch, wenn seine ID+Mode
// mit Hilfe der Maske übereinstimmt
//
// nr = [0...13] (für CAN1)
//--------------------------------------------------------------
void UB_CAN1_std_FilterMask(CAN1_STD_FM_t filter, uint8_t nr)
{
  if(nr>13) nr=0;

  // CAN Filter
  CAN_FilterInitStructure.CAN_FilterNumber = nr;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = P1_make_stdFM(filter,3);     // ID2
  CAN_FilterInitStructure.CAN_FilterIdLow = P1_make_stdFM(filter,1);      // ID1
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P1_make_stdFM(filter,4); // Maske2
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = P1_make_stdFM(filter,2);  // Maske1
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}


//--------------------------------------------------------------
// setzt zwei Filter auf ext_id [29bit] + Mode
// falls ein Filter nicht benötigt wird,
// dann den Filter doppelt anlegen
//
// Ein CAN-Frame kommt nur durch, wenn seine ID+Mode
// in der Liste steht
//
// nr = [0...13] (für CAN1)
//--------------------------------------------------------------
void UB_CAN1_ext_FilterList(CAN1_EXT_FL_t filter, uint8_t nr)
{
  if(nr>13) nr=0;

  // CAN Filter
  CAN_FilterInitStructure.CAN_FilterNumber = nr;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = P1_make_extFL(filter,2);     // ID1_Hi
  CAN_FilterInitStructure.CAN_FilterIdLow = P1_make_extFL(filter,1);      // ID1_Lo
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P1_make_extFL(filter,4); // ID2_Hi
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = P1_make_extFL(filter,3);  // ID2_Lo
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}


//--------------------------------------------------------------
// setzt einen Filter auf ext_id [29bit] + Mode mit Maske
//
// Ein CAN-Frame kommt nur durch, wenn seine ID+Mode
// mit Hilfe der Maske übereinstimmt
//
// nr = [0...13] (für CAN1)
//--------------------------------------------------------------
void UB_CAN1_ext_FilterMask(CAN1_EXT_FM_t filter, uint8_t nr)
{
  if(nr>13) nr=0;

  // CAN Filter
  CAN_FilterInitStructure.CAN_FilterNumber = nr;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = P1_make_extFM(filter,2);     // ID1_Hi
  CAN_FilterInitStructure.CAN_FilterIdLow = P1_make_extFM(filter,1);      // ID1_Lo
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P1_make_extFM(filter,4); // Mask1_Hi
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = P1_make_extFM(filter,3);  // Mask1_Lo
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}



//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P1_make_stdFL(CAN1_STD_FL_t filter, uint8_t nr)
{
  uint16_t ret_wert=0;

  if(nr==1) { // ID1
    ret_wert=(filter.id1.id<<5);
    if((filter.id1.frame_mode==CAN1_STD_REMOTE) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x10;
    }
    if((filter.id1.frame_mode==CAN1_EXT_DATA) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x08;
    }
  }
  if(nr==2) { // ID2
    ret_wert=(filter.id2.id<<5);
    if((filter.id2.frame_mode==CAN1_STD_REMOTE) || (filter.id2.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x10;
    }
    if((filter.id2.frame_mode==CAN1_EXT_DATA) || (filter.id2.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x08;
    }
  }
  if(nr==3) { // ID3
    ret_wert=(filter.id3.id<<5);
    if((filter.id3.frame_mode==CAN1_STD_REMOTE) || (filter.id3.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x10;
    }
    if((filter.id3.frame_mode==CAN1_EXT_DATA) || (filter.id3.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x08;
    }
  }
  if(nr==4) { // ID4
    ret_wert=(filter.id4.id<<5);
    if((filter.id4.frame_mode==CAN1_STD_REMOTE) || (filter.id4.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x10;
    }
    if((filter.id4.frame_mode==CAN1_EXT_DATA) || (filter.id4.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x08;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P1_make_stdFM(CAN1_STD_FM_t filter, uint8_t nr)
{
	uint16_t ret_wert=0;

  if(nr==1) { // ID1
    ret_wert=(filter.id1.id<<5);
    if((filter.id1.frame_mode==CAN1_STD_REMOTE) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x10;
    }
    if((filter.id1.frame_mode==CAN1_EXT_DATA) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x08;
    }
  }
  if(nr==2) { // Maske1
    ret_wert=(filter.mask1.id_mask<<5);
    if(filter.mask1.rtr_bit==CAN1_VALUE_CHECK) {
      ret_wert|=0x10;
    }
    if(filter.mask1.ide_bit==CAN1_VALUE_CHECK) {
      ret_wert|=0x08;
    }
  }
  if(nr==3) { // ID2
    ret_wert=(filter.id2.id<<5);
    if((filter.id2.frame_mode==CAN1_STD_REMOTE) || (filter.id2.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x10;
    }
    if((filter.id2.frame_mode==CAN1_EXT_DATA) || (filter.id2.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x08;
    }
  }
  if(nr==4) { // Maske2
    ret_wert=(filter.mask2.id_mask<<5);
    if(filter.mask2.rtr_bit==CAN1_VALUE_CHECK) {
      ret_wert|=0x10;
    }
    if(filter.mask2.ide_bit==CAN1_VALUE_CHECK) {
      ret_wert|=0x08;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P1_make_extFL(CAN1_EXT_FL_t filter, uint8_t nr)
{
  uint16_t ret_wert=0;
  uint32_t dummy;

  if(nr==1) { // ID1_Lo
    dummy=(filter.id1.id&0x00001FFF);
    ret_wert=(uint16_t)(dummy<<3);
    if((filter.id1.frame_mode==CAN1_EXT_DATA) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x04;
    }
    if((filter.id1.frame_mode==CAN1_STD_REMOTE) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x02;
    }
  }
  if(nr==2) { // ID1_Hi
    dummy=(filter.id1.id&0x1FFFE000);
    ret_wert=(uint16_t)(dummy>>13);
  }
  if(nr==3) { // ID2_Lo
    dummy=(filter.id2.id&0x00001FFF);
    ret_wert=(uint16_t)(dummy<<3);
    if((filter.id2.frame_mode==CAN1_EXT_DATA) || (filter.id2.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x04;
    }
    if((filter.id2.frame_mode==CAN1_STD_REMOTE) || (filter.id2.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x02;
    }
  }
  if(nr==4) { // ID2_Hi
    dummy=(filter.id2.id&0x1FFFE000);
    ret_wert=(uint16_t)(dummy>>13);
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P1_make_extFM(CAN1_EXT_FM_t filter, uint8_t nr)
{
  uint16_t ret_wert=0;
  uint32_t dummy;

  if(nr==1) { // ID1_Lo
    dummy=(filter.id1.id&0x00001FFF);
    ret_wert=(uint16_t)(dummy<<3);
    if((filter.id1.frame_mode==CAN1_EXT_DATA) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x04;
    }
    if((filter.id1.frame_mode==CAN1_STD_REMOTE) || (filter.id1.frame_mode==CAN1_EXT_REMOTE)) {
      ret_wert|=0x02;
    }
  }
  if(nr==2) { // ID1_Hi
    dummy=(filter.id1.id&0x1FFFE000);
    ret_wert=(uint16_t)(dummy>>13);
  }
  if(nr==3) { // Mask1_Lo
    dummy=(filter.mask1.id_mask&0x00001FFF);
    ret_wert=(uint16_t)(dummy<<3);
    if(filter.mask1.ide_bit==CAN1_VALUE_CHECK) {
      ret_wert|=0x04;
    }
    if(filter.mask1.rtr_bit==CAN1_VALUE_CHECK) {
      ret_wert|=0x02;
    }
  }
  if(nr==4) { // Mask1_Hi
    dummy=(filter.mask1.id_mask&0x1FFFE000);
    ret_wert=(uint16_t)(dummy>>13);
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P1_init_NVIC(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// init aller GPIOs
//--------------------------------------------------------------
void P1_init_GPIO(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  // Clock Enable der Pins
  RCC_AHB1PeriphClockCmd(CAN1DEV.TX.CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(CAN1DEV.RX.CLK, ENABLE);

  // CAN Alternative-Funktions mit den IO-Pins verbinden
  GPIO_PinAFConfig(CAN1DEV.TX.PORT, CAN1DEV.TX.SOURCE, GPIO_AF_CAN1);
  GPIO_PinAFConfig(CAN1DEV.RX.PORT, CAN1DEV.RX.SOURCE, GPIO_AF_CAN1);

  // CAN als Alternative-Funktion als Ausgang mit PullUp
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

  // TX-Pin
  GPIO_InitStructure.GPIO_Pin = CAN1DEV.TX.PIN;
  GPIO_Init(CAN1DEV.TX.PORT, &GPIO_InitStructure);
  // RX-Pin
  GPIO_InitStructure.GPIO_Pin = CAN1DEV.RX.PIN;
  GPIO_Init(CAN1DEV.RX.PORT, &GPIO_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// init der CAN-Schnittstelle
//--------------------------------------------------------------
void P1_init_CAN(void)
{
  CAN_InitTypeDef        CAN_InitStructure;
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  uint8_t n;

  // CAN Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

  // CAN deinit
  CAN_DeInit(CAN1);

  // init CAN
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;

  // CAN Baudrate
  CAN_InitStructure.CAN_BS1 = CAN_BS1_14tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;
  CAN_InitStructure.CAN_Prescaler = CAN1_CLOCK_PRESCALER;
  CAN_Init(CAN1, &CAN_InitStructure);

  // CAN Filter (keine CAN-Frames filtern)
  CAN_FilterInitStructure.CAN_FilterNumber = 0; // erste Nr fuer CAN1
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);

  // Init TX-Massage
  CAN1_TxMessage.StdId = 0x00;
  CAN1_TxMessage.ExtId = 0x00;
  CAN1_TxMessage.RTR = CAN_RTR_DATA;
  CAN1_TxMessage.IDE = CAN_ID_STD;
  CAN1_TxMessage.DLC = 0;

  // Init RX-Massage
  CAN1_RxMessage.StdId = 0x00;
  CAN1_RxMessage.ExtId = 0x00;
  CAN1_RxMessage.IDE = CAN_ID_STD;
  CAN1_RxMessage.DLC = 0;
  CAN1_RxMessage.FMI = 0;

  // Init RX-Buffer
  can1_buffer.status=CAN1_RX_EMPTY;
  can1_buffer.frame_mode=CAN1_STD_DATA;
  can1_buffer.can_id=0x00;
  can1_buffer.anz_bytes=0;

  for(n=0;n<8;n++) {
    CAN1_RxMessage.Data[n] = 0x00;
    CAN1_TxMessage.Data[n] = 0x00;
    can1_buffer.data[n] = 0x00;
  }

  // Enable Interrupt
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}


//--------------------------------------------------------------
// interne Funktion
// kopiert empfangenen Frame in Frame_Buffer
//--------------------------------------------------------------
void P1_receive(void)
{
  uint8_t n;

  if(can1_buffer.status==CAN1_RX_EMPTY) {
    can1_buffer.anz_bytes=CAN1_RxMessage.DLC;
    if(CAN1_RxMessage.RTR==CAN_RTR_DATA) {
      if(CAN1_RxMessage.IDE==CAN_ID_STD) {
        can1_buffer.can_id=CAN1_RxMessage.StdId;
        can1_buffer.frame_mode=CAN1_STD_DATA;
      }
      else {
        can1_buffer.can_id=CAN1_RxMessage.ExtId;
        can1_buffer.frame_mode=CAN1_EXT_DATA;
      }
      for(n=0;n<8;n++) {
        if(n<can1_buffer.anz_bytes) {
          can1_buffer.data[n]=CAN1_RxMessage.Data[n];
        }
        else {
          can1_buffer.data[n]=0x00;
        }
      }
    }
    else {
      if(CAN1_RxMessage.IDE==CAN_ID_STD) {  
        can1_buffer.can_id=CAN1_RxMessage.StdId;
        can1_buffer.frame_mode=CAN1_STD_REMOTE;
      }
      else {
        can1_buffer.can_id=CAN1_RxMessage.ExtId;
        can1_buffer.frame_mode=CAN1_EXT_REMOTE;
      }
      for(n=0;n<8;n++) {
        can1_buffer.data[n]=0x00;
      }
    }

    can1_buffer.status=CAN1_RX_READY;
  }
}


//--------------------------------------------------------------
// ISR-Funktion von CAN1-RX
// wird aufgerufen, wenn ein Frame per CAN empfangen wurde
//--------------------------------------------------------------
void CAN1_RX0_IRQHandler(void)
{
  CAN_Receive(CAN1, CAN_FIFO0, &CAN1_RxMessage);

  P1_receive();
}
