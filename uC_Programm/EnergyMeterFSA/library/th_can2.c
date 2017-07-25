//--------------------------------------------------------------
// File     : th_can.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / CAN / MISC
// Funktion : CAN2
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_can2.h"
#include "main.h"

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P2_init_NVIC(void);
void P2_init_GPIO(void);
void P2_init_CAN(void);
void P2_receive(void);

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
void TH_CAN2_Init(void) {
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
ErrorStatus TH_CAN2_send_std_data(CAN2_TX_FRAME_t tx_frame) {
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
CAN2_STATUS_t TH_CAN2_receive(CAN2_RX_FRAME_t *rx_frame) {
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
	static int counter = 0;

	P2_receive();

	if (state != AUSLESEN) {
		if (CAN2_RxMessage.StdId == 0x521) { // Current
			current = (CAN2_RxMessage.Data[2] * 0x1000000)
					+ (CAN2_RxMessage.Data[3] * 0x10000)
					+ (CAN2_RxMessage.Data[4] * 0x100)
					+ (CAN2_RxMessage.Data[5]);
			if (state == LOGGING) {
				saveData();
			}
		}
		if (CAN2_RxMessage.StdId == 0x522) { // Voltage
			voltage = (CAN2_RxMessage.Data[2] * 0x1000000)
					+ (CAN2_RxMessage.Data[3] * 0x10000)
					+ (CAN2_RxMessage.Data[4] * 0x100)
					+ (CAN2_RxMessage.Data[5]);
			/*if (canstate) {
				counter++;
				if (counter > 9) {
					sendCAN();
					counter = 0;
				}
			}*/
		}
		if (CAN2_RxMessage.StdId == 0x525) { // Temperature
			temperature = (CAN2_RxMessage.Data[2] * 0x1000000)
					+ (CAN2_RxMessage.Data[3] * 0x10000)
					+ (CAN2_RxMessage.Data[4] * 0x100)
					+ (CAN2_RxMessage.Data[5]);
		}
	}
	interrupttimer = 0;
}
