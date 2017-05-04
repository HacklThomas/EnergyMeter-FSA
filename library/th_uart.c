//--------------------------------------------------------------
// File     : th_uart.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / USART / MISC
// Funktion : UART - Bluetooth Schnittstelle
//--------------------------------------------------------------
// Pinbelegung: TX: PC10 / RX: PC11 -> UART3
//
// Als Endekennung beim Empfangen, muss der Sender das Zeichen
// "0x0D" = Carriage-Return an den String anhaengen !!
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_uart.h"
#include "main.h"

//--------------------------------------------------------------
// Definition aller UARTs
// Reihenfolge wie bei UART_NAME_t
//--------------------------------------------------------------
UART_t UART[UART_ANZ] = { COM1, RCC_APB1Periph_USART3, GPIO_AF_USART3, USART3,
		9600, USART3_IRQn, { GPIOC, GPIO_Pin_10, RCC_AHB1Periph_GPIOC,
				GPIO_PinSource10 }, { GPIOC, GPIO_Pin_11, RCC_AHB1Periph_GPIOC,
				GPIO_PinSource11 } };

//--------------------------------------------------------------
// init aller UARTs
//--------------------------------------------------------------
void TH_UART_Init(int secondinit) {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	UART_NAME_t nr;

	if (secondinit == 1) {
		UART[0].BAUD = 230400;
	} else {
		UART[0].BAUD = 115200;
	}

	for (nr = 0; nr < UART_ANZ; nr++) {

		// Clock enable der TX und RX Pins
		RCC_AHB1PeriphClockCmd(UART[nr].TX.CLK, ENABLE);
		RCC_AHB1PeriphClockCmd(UART[nr].RX.CLK, ENABLE);

		// Clock enable der UART
		if ((UART[nr].UART == USART1) || (UART[nr].UART == USART6)) {
			RCC_APB2PeriphClockCmd(UART[nr].CLK, ENABLE);
		} else {
			RCC_APB1PeriphClockCmd(UART[nr].CLK, ENABLE);
		}

		// UART Alternative-Funktions mit den IO-Pins verbinden
		GPIO_PinAFConfig(UART[nr].TX.PORT, UART[nr].TX.SOURCE, UART[nr].AF);
		GPIO_PinAFConfig(UART[nr].RX.PORT, UART[nr].RX.SOURCE, UART[nr].AF);

		// UART als Alternative-Funktion mit PushPull
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		// TX-Pin
		GPIO_InitStructure.GPIO_Pin = UART[nr].TX.PIN;
		GPIO_Init(UART[nr].TX.PORT, &GPIO_InitStructure);
		// RX-Pin
		GPIO_InitStructure.GPIO_Pin = UART[nr].RX.PIN;
		GPIO_Init(UART[nr].RX.PORT, &GPIO_InitStructure);

		// Oversampling
		USART_OverSampling8Cmd(UART[nr].UART, ENABLE);

		// init mit Baudrate, 8Databits, 1Stopbit, keine Paritaet, kein RTS+CTS
		USART_InitStructure.USART_BaudRate = UART[nr].BAUD;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(UART[nr].UART, &USART_InitStructure);

		// UART enable
		USART_Cmd(UART[nr].UART, ENABLE);

		// RX-Interrupt enable
		USART_ITConfig(UART[nr].UART, USART_IT_RXNE, ENABLE);

		// enable UART Interrupt-Vector
		NVIC_InitStructure.NVIC_IRQChannel = UART[nr].INT;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		// RX-Puffer vorbereiten
		UART_RX[nr].rx_buffer[0] = RX_END_CHR;
		UART_RX[nr].wr_ptr = 0;
		UART_RX[nr].rd_ptr = 0;
		UART_RX[nr].status = RX_EMPTY;
	}
}

//--------------------------------------------------------------
// ein Byte per UART senden
//--------------------------------------------------------------
void TH_UART_SendByte(UART_NAME_t uart, uint16_t wert) {
	// warten bis altes Byte gesendet wurde
	while (USART_GetFlagStatus(UART[uart].UART, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(UART[uart].UART, wert);
}

//--------------------------------------------------------------
// einen String per UART senden
//--------------------------------------------------------------
void TH_UART_SendString(UART_NAME_t uart, char *ptr, UART_LASTBYTE_t end_cmd) {
	// sende kompletten String
	while (*ptr != 0) {
		TH_UART_SendByte(uart, *ptr);
		ptr++;
	}
	// eventuell Endekennung senden
	if (end_cmd == LFCR) {
		TH_UART_SendByte(uart, 0x0A); // LineFeed senden
		TH_UART_SendByte(uart, 0x0D); // CariageReturn senden
	} else if (end_cmd == CRLF) {
		TH_UART_SendByte(uart, 0x0D); // CariageReturn senden
		TH_UART_SendByte(uart, 0x0A); // LineFeed senden
	} else if (end_cmd == LF) {
		TH_UART_SendByte(uart, 0x0A); // LineFeed senden
	} else if (end_cmd == CR) {
		TH_UART_SendByte(uart, 0x0D); // CariageReturn senden
	}
}

//--------------------------------------------------------------
// einen String per UART empfangen
// (der Empfang wird per Interrupt abgehandelt)
// diese Funktion muss zyklisch gepollt werden
// Return Wert :
//  -> wenn nichts empfangen = RX_EMPTY
//  -> wenn String empfangen = RX_READY -> String steht in *ptr
//  -> wenn Puffer voll      = RX_FULL
//--------------------------------------------------------------
UART_RXSTATUS_t TH_UART_ReceiveString(UART_NAME_t uart, char *ptr) {
	UART_RXSTATUS_t ret_wert = RX_EMPTY;
	uint16_t rd_pos, wr_pos, n;
	uint8_t wert, ok;

	// aktuelle Pointer auslesen
	rd_pos = UART_RX[uart].rd_ptr;
	wr_pos = UART_RX[uart].wr_ptr;
	// check ob daten im puffer
	ok = 0;
	if (rd_pos != wr_pos) {
		// Endekennung suchen
		while (rd_pos != wr_pos) {
			wert = UART_RX[uart].rx_buffer[rd_pos];
			if (wert == RX_END_CHR) {
				// Endekennung gefunden
				ok = 1;
				break;
			}
			// lese pointer weiterschieben
			rd_pos++;
			if (rd_pos >= RX_BUF_SIZE)
				rd_pos = 0;
		}

		// falls Endekennung gefunden, String auslesen
		if (ok == 1) {
			// endekennung gefunden
			ret_wert = RX_READY;
			// string komplett auslesen
			rd_pos = UART_RX[uart].rd_ptr;
			n = 0;
			while (rd_pos != wr_pos) {
				wert = UART_RX[uart].rx_buffer[rd_pos];
				// lese pointer weiterschieben
				rd_pos++;
				if (rd_pos >= RX_BUF_SIZE)
					rd_pos = 0;
				// nur Ascii-Zeichen speichern
				if ((wert >= RX_FIRST_CHR) && (wert <= RX_LAST_CHR)) {
					ptr[n] = wert;
					n++;
				} else if (wert == RX_END_CHR) {
					break;
				}
			}
			// Stringendekennung
			ptr[n] = 0x00;
			// lese pointer aktuallisieren
			UART_RX[uart].rd_ptr = rd_pos;
		} else {
			// keine Endekennung gefunden
			ret_wert = RX_EMPTY;
		}
	} else {
		// check ob ueberlauf aufgetreten
		if (UART_RX[uart].status == RX_FULL) {
			ret_wert = RX_FULL;
			UART_RX[uart].status = RX_EMPTY;
		}
	}

	return (ret_wert);
}

//--------------------------------------------------------------
// ein Daten Array per UART empfangen
// Return Wert :
//   Anzahl der bis jetzt empfangenen Bytes [0...RX_BUF_SIZE]
//--------------------------------------------------------------
uint32_t TH_UART_ReceiveArray(UART_NAME_t uart, uint8_t *data) {
	uint32_t n = 0;
	uint16_t rd_pos, wr_pos;
	uint8_t wert;

	// aktuelle Pointer auslesen
	rd_pos = UART_RX[uart].rd_ptr;
	wr_pos = UART_RX[uart].wr_ptr;
	// check ob daten im puffer
	n = 0;
	if (rd_pos != wr_pos) {
		// alle daten auslesen
		while (rd_pos != wr_pos) {
			wert = UART_RX[uart].rx_buffer[rd_pos];
			// lese pointer weiterschieben
			rd_pos++;
			if (rd_pos >= RX_BUF_SIZE)
				rd_pos = 0;
			// daten uebergeben
			data[n] = wert;
			n++;
		}
		// lese pointer aktualisieren
		UART_RX[uart].rd_ptr = rd_pos;
	}

	return (n);
}

//--------------------------------------------------------------
// interne Funktion
// speichern des empfangenen Zeichens im Puffer
//--------------------------------------------------------------
void P_UART_Receive(UART_NAME_t uart, uint16_t wert) {
	uint16_t wr_pos;

	// byte an schreib position speichern
	wr_pos = UART_RX[uart].wr_ptr;

	UART_RX[uart].rx_buffer[wr_pos] = wert;
	// schreib pointer weiterschieben
	wr_pos++;
	if (wr_pos >= RX_BUF_SIZE)
		wr_pos = 0;
	UART_RX[uart].wr_ptr = wr_pos;

	// check ob Ueberlauf
	if (UART_RX[uart].wr_ptr == UART_RX[uart].rd_ptr) {
		UART_RX[uart].status = RX_FULL;
	}
}

//--------------------------------------------------------------
// interne Funktion
// UART-Interrupt-Funktion
// Interrupt-Nr muss uebergeben werden
//--------------------------------------------------------------
void P_UART_RX_INT(uint8_t int_nr, uint16_t wert) {
	UART_NAME_t nr;

	// den passenden Eintrag suchen
	for (nr = 0; nr < UART_ANZ; nr++) {
		if (UART[nr].INT == int_nr) {
			// eintrag gefunden, Byte speichern
			P_UART_Receive(nr, wert);
			break;
		}
	}
}

//--------------------------------------------------------------
// UART3-Interrupt
//--------------------------------------------------------------
void USART3_IRQHandler(void) {
	uint16_t wert;

	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
		// wenn ein Byte im Empfangspuffer steht
		wert = USART_ReceiveData(USART3);
		// Byte speichern
		P_UART_RX_INT(USART3_IRQn, wert);
	}
	hcreceive = TRUE;
}
