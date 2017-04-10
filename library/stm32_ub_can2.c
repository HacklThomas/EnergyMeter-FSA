//--------------------------------------------------------------
// File     : stm32_ub_can2.c
// Datum    : 21.09.2013
// Version  : 1.2
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, CAN, MISC
// Funktion : CAN-LoLevel-Funktionen (per CAN-2)
//            externer CAN-Transeiver wird benötigt
//
// Hinweis  : mögliche Pinbelegungen
//            CAN2 : TX:[PB6, PB13]
//                   RX:[PB5, PB12]
//
// Terminations-Widerstand (120 Ohm) am CAN-Bus nicht vergessen !
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_can2.h"
#include "main.h"

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P2_init_NVIC(void);
void P2_init_GPIO(void);
void P2_init_CAN(void);
void P2_receive(void);
uint16_t P2_make_stdFL(CAN2_STD_FL_t filter, uint8_t nr);
uint16_t P2_make_stdFM(CAN2_STD_FM_t filter, uint8_t nr);
uint16_t P2_make_extFL(CAN2_EXT_FL_t filter, uint8_t nr);
uint16_t P2_make_extFM(CAN2_EXT_FM_t filter, uint8_t nr);

//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
CAN_FilterInitTypeDef CAN_FilterInitStructure;
CanRxMsg CAN2_RxMessage;
CanTxMsg CAN2_TxMessage;
CAN2_BUFFER_t can2_buffer;

//--------------------------------------------------------------
// Definition von CAN2
//--------------------------------------------------------------
CAN2_DEV_t CAN2DEV = {
// PORT , PIN      , Clock              , Source
		{ GPIOB, GPIO_Pin_6, RCC_AHB1Periph_GPIOB, GPIO_PinSource6 }, // TX an PB6
		{ GPIOB, GPIO_Pin_5, RCC_AHB1Periph_GPIOB, GPIO_PinSource5 }, // RX an PB5
		};

//--------------------------------------------------------------
// Init von CAN2
//--------------------------------------------------------------
void UB_CAN2_Init(void) {
	static uint8_t init_ok = 0;

	// initialisierung darf nur einmal gemacht werden
	if (init_ok != 0) {
		return;
	}

	// init vom NVIC
	P2_init_NVIC();

	// init der GPIO
	P2_init_GPIO();

	// init vom CAN
	P2_init_CAN();

	// init Mode speichern
	init_ok = 1;
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
ErrorStatus UB_CAN2_send_std_data(CAN2_TX_FRAME_t tx_frame) {
	uint8_t n;
	uint32_t timeout;

	if (tx_frame.can_id > 0x7FF) {
		return (ERROR);
	}

	if (tx_frame.anz_bytes > 8) {
		return (ERROR);
	}

	// daten frame vorbereiten
	CAN2_TxMessage.StdId = tx_frame.can_id;
	CAN2_TxMessage.ExtId = 0x00;
	CAN2_TxMessage.RTR = CAN_RTR_DATA;
	CAN2_TxMessage.IDE = CAN_ID_STD;
	CAN2_TxMessage.DLC = tx_frame.anz_bytes;

	for (n = 0; n < tx_frame.anz_bytes; n++) {
		CAN2_TxMessage.Data[n] = tx_frame.data[n];
	}

	// warten bis ein Transmit Register frei ist
	timeout = 0;
	while (!(CAN2->TSR & CAN_TSR_TME0 || CAN2->TSR & CAN_TSR_TME1
			|| CAN2->TSR & CAN_TSR_TME2)) {
		timeout++;
		if (timeout > CAN2_TX_TIMEOUT)
			break;
	}

	// daten frame senden
	CAN_Transmit(CAN2, &CAN2_TxMessage);

	return (SUCCESS);
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
ErrorStatus UB_CAN2_send_std_remote(CAN2_TX_FRAME_t tx_frame) {
	uint32_t timeout;

	if (tx_frame.can_id > 0x7FF) {
		return (ERROR);
	}

	if (tx_frame.anz_bytes > 8) {
		return (ERROR);
	}

	// remote frame vorbereiten
	CAN2_TxMessage.StdId = tx_frame.can_id;
	CAN2_TxMessage.ExtId = 0x00;
	CAN2_TxMessage.RTR = CAN_RTR_REMOTE;
	CAN2_TxMessage.IDE = CAN_ID_STD;
	CAN2_TxMessage.DLC = tx_frame.anz_bytes;

	// warten bis ein Transmit Register frei ist
	timeout = 0;
	while (!(CAN2->TSR & CAN_TSR_TME0 || CAN2->TSR & CAN_TSR_TME1
			|| CAN2->TSR & CAN_TSR_TME2)) {
		timeout++;
		if (timeout > CAN2_TX_TIMEOUT)
			break;
	}

	// remote frame senden
	CAN_Transmit(CAN2, &CAN2_TxMessage);

	return (SUCCESS);
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
ErrorStatus UB_CAN2_send_ext_data(CAN2_TX_FRAME_t tx_frame) {
	uint8_t n;
	uint32_t timeout;

	if (tx_frame.can_id > 0x1FFFFFFF) {
		return (ERROR);
	}

	if (tx_frame.anz_bytes > 8) {
		return (ERROR);
	}

	// daten frame vorbereiten
	CAN2_TxMessage.StdId = 0x00;
	CAN2_TxMessage.ExtId = tx_frame.can_id;
	CAN2_TxMessage.RTR = CAN_RTR_DATA;
	CAN2_TxMessage.IDE = CAN_ID_EXT;
	CAN2_TxMessage.DLC = tx_frame.anz_bytes;

	for (n = 0; n < tx_frame.anz_bytes; n++) {
		CAN2_TxMessage.Data[n] = tx_frame.data[n];
	}

	// warten bis ein Transmit Register frei ist
	timeout = 0;
	while (!(CAN2->TSR & CAN_TSR_TME0 || CAN2->TSR & CAN_TSR_TME1
			|| CAN2->TSR & CAN_TSR_TME2)) {
		timeout++;
		if (timeout > CAN2_TX_TIMEOUT)
			break;
	}

	// daten frame senden
	CAN_Transmit(CAN2, &CAN2_TxMessage);

	return (SUCCESS);
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
ErrorStatus UB_CAN2_send_ext_remote(CAN2_TX_FRAME_t tx_frame) {
	uint32_t timeout;

	if (tx_frame.can_id > 0x1FFFFFFF) {
		return (ERROR);
	}

	if (tx_frame.anz_bytes > 8) {
		return (ERROR);
	}

	// remote frame vorbereiten
	CAN2_TxMessage.StdId = 0x00;
	CAN2_TxMessage.ExtId = tx_frame.can_id;
	CAN2_TxMessage.RTR = CAN_RTR_REMOTE;
	CAN2_TxMessage.IDE = CAN_ID_EXT;
	CAN2_TxMessage.DLC = tx_frame.anz_bytes;

	// warten bis ein Transmit Register frei ist
	timeout = 0;
	while (!(CAN2->TSR & CAN_TSR_TME0 || CAN2->TSR & CAN_TSR_TME1
			|| CAN2->TSR & CAN_TSR_TME2)) {
		timeout++;
		if (timeout > CAN2_TX_TIMEOUT)
			break;
	}

	// remote frame senden
	CAN_Transmit(CAN2, &CAN2_TxMessage);

	return (SUCCESS);
}

//--------------------------------------------------------------
// check ob CAN-Frame empfangen wurde
// return_wert :
//    CAN2_RX_EMPTY = nichts empfangen
//    CAN2_RX_READY = CAN-Frame empfangen
//
// rx_frame :
// frame_mode = [CAN2_STD_DATA,CAN2_STD_REMOTE,CAN2_EXT_DATA,CAN2_EXT_REMOTE]
// can_id     = Message Identifier
// anz_bytes  = Anzahl der Daten
// data[]     = Daten die empfangen wurden (nur bei Data-Frame)
//--------------------------------------------------------------
CAN2_STATUS_t UB_CAN2_receive(CAN2_RX_FRAME_t *rx_frame) {
	CAN2_STATUS_t ret_wert = CAN2_RX_EMPTY;
	uint8_t n;

	if (can2_buffer.status == CAN2_RX_READY) {
		// wenn Frame empfangen wurde -> inhalt kopieren
		rx_frame->frame_mode = can2_buffer.frame_mode;
		rx_frame->can_id = can2_buffer.can_id;
		rx_frame->anz_bytes = can2_buffer.anz_bytes;
		for (n = 0; n < 8; n++) {
			rx_frame->data[n] = can2_buffer.data[n];
		}

		ret_wert = CAN2_RX_READY;
		can2_buffer.status = CAN2_RX_EMPTY;
	}

	return (ret_wert);
}

//--------------------------------------------------------------
// setzt vier Filter auf std_id [11bit] + Mode
// falls ein Filter nicht benötigt wird,
// dann den Filter doppelt anlegen
//
// Ein CAN-Frame kommt nur durch, wenn seine ID+Mode
// in der Liste steht
//
// nr = [14...27] (für CAN2)
//--------------------------------------------------------------
void UB_CAN2_std_FilterList(CAN2_STD_FL_t filter, uint8_t nr) {
	if (nr < 14)
		nr = 14;
	if (nr > 27)
		nr = 14;

	// CAN Filter
	CAN_FilterInitStructure.CAN_FilterNumber = nr;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = P2_make_stdFL(filter, 2); // ID2
	CAN_FilterInitStructure.CAN_FilterIdLow = P2_make_stdFL(filter, 1); // ID1
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P2_make_stdFL(filter, 4); // ID4
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = P2_make_stdFL(filter, 3); // ID3
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
// nr = [14...27] (für CAN2)
//--------------------------------------------------------------
void UB_CAN2_std_FilterMask(CAN2_STD_FM_t filter, uint8_t nr) {
	if (nr < 14)
		nr = 14;
	if (nr > 27)
		nr = 14;

	// CAN Filter
	CAN_FilterInitStructure.CAN_FilterNumber = nr;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = P2_make_stdFM(filter, 3); // ID2
	CAN_FilterInitStructure.CAN_FilterIdLow = P2_make_stdFM(filter, 1); // ID1
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P2_make_stdFM(filter, 4); // Maske2
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = P2_make_stdFM(filter, 2); // Maske1
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
// nr = [14...27] (für CAN2)
//--------------------------------------------------------------
void UB_CAN2_ext_FilterList(CAN2_EXT_FL_t filter, uint8_t nr) {
	if (nr < 14)
		nr = 14;
	if (nr > 27)
		nr = 14;

	// CAN Filter
	CAN_FilterInitStructure.CAN_FilterNumber = nr;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = P2_make_extFL(filter, 2); // ID1_Hi
	CAN_FilterInitStructure.CAN_FilterIdLow = P2_make_extFL(filter, 1); // ID1_Lo
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P2_make_extFL(filter, 4); // ID2_Hi
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = P2_make_extFL(filter, 3); // ID2_Lo
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
// nr = [14...27] (für CAN2)
//--------------------------------------------------------------
void UB_CAN2_ext_FilterMask(CAN2_EXT_FM_t filter, uint8_t nr) {
	if (nr < 14)
		nr = 14;
	if (nr > 27)
		nr = 14;

	// CAN Filter
	CAN_FilterInitStructure.CAN_FilterNumber = nr;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = P2_make_extFM(filter, 2); // ID1_Hi
	CAN_FilterInitStructure.CAN_FilterIdLow = P2_make_extFM(filter, 1); // ID1_Lo
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = P2_make_extFM(filter, 4); // Mask1_Hi
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = P2_make_extFM(filter, 3); // Mask1_Lo
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
}

//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P2_make_stdFL(CAN2_STD_FL_t filter, uint8_t nr) {
	uint16_t ret_wert = 0;

	if (nr == 1) { // ID1
		ret_wert = (filter.id1.id << 5);
		if ((filter.id1.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x10;
		}
		if ((filter.id1.frame_mode == CAN2_EXT_DATA)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x08;
		}
	}
	if (nr == 2) { // ID2
		ret_wert = (filter.id2.id << 5);
		if ((filter.id2.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id2.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x10;
		}
		if ((filter.id2.frame_mode == CAN2_EXT_DATA)
				|| (filter.id2.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x08;
		}
	}
	if (nr == 3) { // ID3
		ret_wert = (filter.id3.id << 5);
		if ((filter.id3.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id3.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x10;
		}
		if ((filter.id3.frame_mode == CAN2_EXT_DATA)
				|| (filter.id3.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x08;
		}
	}
	if (nr == 4) { // ID4
		ret_wert = (filter.id4.id << 5);
		if ((filter.id4.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id4.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x10;
		}
		if ((filter.id4.frame_mode == CAN2_EXT_DATA)
				|| (filter.id4.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x08;
		}
	}

	return (ret_wert);
}

//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P2_make_stdFM(CAN2_STD_FM_t filter, uint8_t nr) {
	uint16_t ret_wert = 0;

	if (nr == 1) { // ID1
		ret_wert = (filter.id1.id << 5);
		if ((filter.id1.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x10;
		}
		if ((filter.id1.frame_mode == CAN2_EXT_DATA)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x08;
		}
	}
	if (nr == 2) { // Maske1
		ret_wert = (filter.mask1.id_mask << 5);
		if (filter.mask1.rtr_bit == CAN2_VALUE_CHECK) {
			ret_wert |= 0x10;
		}
		if (filter.mask1.ide_bit == CAN2_VALUE_CHECK) {
			ret_wert |= 0x08;
		}
	}
	if (nr == 3) { // ID2
		ret_wert = (filter.id2.id << 5);
		if ((filter.id2.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id2.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x10;
		}
		if ((filter.id2.frame_mode == CAN2_EXT_DATA)
				|| (filter.id2.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x08;
		}
	}
	if (nr == 4) { // Maske2
		ret_wert = (filter.mask2.id_mask << 5);
		if (filter.mask2.rtr_bit == CAN2_VALUE_CHECK) {
			ret_wert |= 0x10;
		}
		if (filter.mask2.ide_bit == CAN2_VALUE_CHECK) {
			ret_wert |= 0x08;
		}
	}

	return (ret_wert);
}

//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P2_make_extFL(CAN2_EXT_FL_t filter, uint8_t nr) {
	uint16_t ret_wert = 0;
	uint32_t dummy;

	if (nr == 1) { // ID1_Lo
		dummy = (filter.id1.id & 0x00001FFF);
		ret_wert = (uint16_t)(dummy << 3);
		if ((filter.id1.frame_mode == CAN2_EXT_DATA)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x04;
		}
		if ((filter.id1.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x02;
		}
	}
	if (nr == 2) { // ID1_Hi
		dummy = (filter.id1.id & 0x1FFFE000);
		ret_wert = (uint16_t)(dummy >> 13);
	}
	if (nr == 3) { // ID2_Lo
		dummy = (filter.id2.id & 0x00001FFF);
		ret_wert = (uint16_t)(dummy << 3);
		if ((filter.id2.frame_mode == CAN2_EXT_DATA)
				|| (filter.id2.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x04;
		}
		if ((filter.id2.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id2.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x02;
		}
	}
	if (nr == 4) { // ID2_Hi
		dummy = (filter.id2.id & 0x1FFFE000);
		ret_wert = (uint16_t)(dummy >> 13);
	}

	return (ret_wert);
}

//--------------------------------------------------------------
// interne Funktion
// nr [1...4]
//--------------------------------------------------------------
uint16_t P2_make_extFM(CAN2_EXT_FM_t filter, uint8_t nr) {
	uint16_t ret_wert = 0;
	uint32_t dummy;

	if (nr == 1) { // ID1_Lo
		dummy = (filter.id1.id & 0x00001FFF);
		ret_wert = (uint16_t)(dummy << 3);
		if ((filter.id1.frame_mode == CAN2_EXT_DATA)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x04;
		}
		if ((filter.id1.frame_mode == CAN2_STD_REMOTE)
				|| (filter.id1.frame_mode == CAN2_EXT_REMOTE)) {
			ret_wert |= 0x02;
		}
	}
	if (nr == 2) { // ID1_Hi
		dummy = (filter.id1.id & 0x1FFFE000);
		ret_wert = (uint16_t)(dummy >> 13);
	}
	if (nr == 3) { // Mask1_Lo
		dummy = (filter.mask1.id_mask & 0x00001FFF);
		ret_wert = (uint16_t)(dummy << 3);
		if (filter.mask1.ide_bit == CAN2_VALUE_CHECK) {
			ret_wert |= 0x04;
		}
		if (filter.mask1.rtr_bit == CAN2_VALUE_CHECK) {
			ret_wert |= 0x02;
		}
	}
	if (nr == 4) { // Mask1_Hi
		dummy = (filter.mask1.id_mask & 0x1FFFE000);
		ret_wert = (uint16_t)(dummy >> 13);
	}

	return (ret_wert);
}

//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P2_init_NVIC(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//--------------------------------------------------------------
// interne Funktion
// init aller GPIOs
//--------------------------------------------------------------
void P2_init_GPIO(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clock Enable der Pins
	RCC_AHB1PeriphClockCmd(CAN2DEV.TX.CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(CAN2DEV.RX.CLK, ENABLE);

	// CAN Alternative-Funktions mit den IO-Pins verbinden
	GPIO_PinAFConfig(CAN2DEV.TX.PORT, CAN2DEV.TX.SOURCE, GPIO_AF_CAN2);
	GPIO_PinAFConfig(CAN2DEV.RX.PORT, CAN2DEV.RX.SOURCE, GPIO_AF_CAN2);

	// CAN als Alternative-Funktion als Ausgang mit PullUp
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	// TX-Pin
	GPIO_InitStructure.GPIO_Pin = CAN2DEV.TX.PIN;
	GPIO_Init(CAN2DEV.TX.PORT, &GPIO_InitStructure);
	// RX-Pin
	GPIO_InitStructure.GPIO_Pin = CAN2DEV.RX.PIN;
	GPIO_Init(CAN2DEV.RX.PORT, &GPIO_InitStructure);
}

//--------------------------------------------------------------
// interne Funktion
// init der CAN-Schnittstelle
//--------------------------------------------------------------
void P2_init_CAN(void) {
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	uint8_t n;

	// CAN Clock enable (bei CAN2 muss CAN1 auch enabled werden !!)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_CAN2, ENABLE);

	// CAN deinit
	CAN_DeInit(CAN2);

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
	CAN_InitStructure.CAN_Prescaler = CAN2_CLOCK_PRESCALER;
	CAN_Init(CAN2, &CAN_InitStructure);

	// CAN Filter (keine CAN-Frames filtern)
	CAN_FilterInitStructure.CAN_FilterNumber = 14; // erste Nr fuer CAN2
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
	CAN2_TxMessage.StdId = 0x00;
	CAN2_TxMessage.ExtId = 0x00;
	CAN2_TxMessage.RTR = CAN_RTR_DATA;
	CAN2_TxMessage.IDE = CAN_ID_STD;
	CAN2_TxMessage.DLC = 0;

	// Init RX-Massage
	CAN2_RxMessage.StdId = 0x00;
	CAN2_RxMessage.ExtId = 0x00;
	CAN2_RxMessage.IDE = CAN_ID_STD;
	CAN2_RxMessage.DLC = 0;
	CAN2_RxMessage.FMI = 0;

	// Init RX-Buffer
	can2_buffer.status = CAN2_RX_EMPTY;
	can2_buffer.frame_mode = CAN2_STD_DATA;
	can2_buffer.can_id = 0x00;
	can2_buffer.anz_bytes = 0;

	for (n = 0; n < 8; n++) {
		CAN2_RxMessage.Data[n] = 0x00;
		CAN2_TxMessage.Data[n] = 0x00;
		can2_buffer.data[n] = 0x00;
	}

	// Enable Interrupt
	CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);
}

//--------------------------------------------------------------
// interne Funktion
// kopiert empfangenen Frame in Frame_Buffer
//--------------------------------------------------------------
void P2_receive(void) {
	uint8_t n;

	if (can2_buffer.status == CAN2_RX_EMPTY) {
		can2_buffer.anz_bytes = CAN2_RxMessage.DLC;
		if (CAN2_RxMessage.RTR == CAN_RTR_DATA) {
			if (CAN2_RxMessage.IDE == CAN_ID_STD) {
				can2_buffer.can_id = CAN2_RxMessage.StdId;
				can2_buffer.frame_mode = CAN2_STD_DATA;
			} else {
				can2_buffer.can_id = CAN2_RxMessage.ExtId;
				can2_buffer.frame_mode = CAN2_EXT_DATA;
			}
			for (n = 0; n < 8; n++) {
				if (n < can2_buffer.anz_bytes) {
					can2_buffer.data[n] = CAN2_RxMessage.Data[n];
				} else {
					can2_buffer.data[n] = 0x00;
				}
			}
		} else {
			if (CAN2_RxMessage.IDE == CAN_ID_STD) {
				can2_buffer.can_id = CAN2_RxMessage.StdId;
				can2_buffer.frame_mode = CAN2_STD_REMOTE;
			} else {
				can2_buffer.can_id = CAN2_RxMessage.ExtId;
				can2_buffer.frame_mode = CAN2_EXT_REMOTE;
			}
			for (n = 0; n < 8; n++) {
				can2_buffer.data[n] = 0x00;
			}
		}

		can2_buffer.status = CAN2_RX_READY;
	}
}

//--------------------------------------------------------------
// ISR-Funktion von CAN2-RX
// wird aufgerufen, wenn ein Frame per CAN empfangen wurde
//--------------------------------------------------------------
void CAN2_RX0_IRQHandler(void) {
	CAN_Receive(CAN2, CAN_FIFO0, &CAN2_RxMessage);

	P2_receive();

	if (CAN2_RxMessage.StdId == 0x521) { // Current
		current = (CAN2_RxMessage.Data[2] * 0x1000000)
				+ (CAN2_RxMessage.Data[3] * 0x10000)
				+ (CAN2_RxMessage.Data[4] * 0x100) + (CAN2_RxMessage.Data[5]);
	}
	if (CAN2_RxMessage.StdId == 0x522) { // Voltage
		voltage = (CAN2_RxMessage.Data[2] * 0x1000000)
				+ (CAN2_RxMessage.Data[3] * 0x10000)
				+ (CAN2_RxMessage.Data[4] * 0x100) + (CAN2_RxMessage.Data[5]);
	}
	if (CAN2_RxMessage.StdId == 0x525) { // Temperature
		temperature = (CAN2_RxMessage.Data[2] * 0x1000000)
				+ (CAN2_RxMessage.Data[3] * 0x10000)
				+ (CAN2_RxMessage.Data[4] * 0x100) + (CAN2_RxMessage.Data[5]);
	}
	if(state == LOGGING){
		saveData();
	}
}
