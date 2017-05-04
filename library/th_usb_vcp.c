//--------------------------------------------------------------
// File     : th_usb_vcp.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / RCC / EXTI / MISC / USB CDC DEVICE
// Funktion : USB als Virtual COM Port
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "th_usb_vcp.h"
#include "usbd_usr.h"

//--------------------------------------------------------------
// Private Funktionen
//--------------------------------------------------------------
uint8_t TH_INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];
uint32_t th_int_usb_vcp_buf_in, th_int_usb_vcp_buf_out, th_int_usb_vcp_buf_num;
extern TH_USB_VCP_Result TH_USB_VCP_INT_Status;
extern LINE_CODING linecoding;
uint8_t TH_USB_VCP_INT_Init = 0;
USB_OTG_CORE_HANDLE	USB_OTG_dev;

/* USB VCP Internal receive buffer */
extern uint8_t TH_INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];

TH_USB_VCP_Result TH_USB_VCP_Init(void) {
	/* Initialize USB */
	USBD_Init(	&USB_OTG_dev,
			USB_OTG_FS_CORE_ID,
				&USR_desc,
				&USBD_CDC_cb,
				&USR_cb);

	/* Reset buffer counters */
	th_int_usb_vcp_buf_in = 0;
	th_int_usb_vcp_buf_out = 0;
	th_int_usb_vcp_buf_num = 0;

	/* Initialized */
	TH_USB_VCP_INT_Init = 1;

	/* Return OK */
	return TH_USB_VCP_OK;
}

uint8_t TH_USB_VCP_BufferEmpty(void) {
	return (th_int_usb_vcp_buf_num == 0);
}

uint8_t TH_USB_VCP_BufferFull(void) {
	return (th_int_usb_vcp_buf_num == USB_VCP_RECEIVE_BUFFER_LENGTH);
}

uint8_t TH_USB_VCP_FindCharacter(volatile char c) {
	uint16_t num, out;

	/* Temp variables */
	num = th_int_usb_vcp_buf_num;
	out = th_int_usb_vcp_buf_out;

	while (num > 0) {
		/* Check overflow */
		if (out == USB_VCP_RECEIVE_BUFFER_LENGTH) {
			out = 0;
		}
		if (TH_INT_USB_VCP_ReceiveBuffer[out] == c) {
			/* Character found */
			return 1;
		}
		out++;
		num--;
	}

	/* Character is not in buffer */
	return 0;
}

TH_USB_VCP_Result TH_USB_VCP_Getc(uint8_t* c) {
	/* Any data in buffer */
	if (th_int_usb_vcp_buf_num > 0) {
		/* Check overflow */
		if (th_int_usb_vcp_buf_out >= USB_VCP_RECEIVE_BUFFER_LENGTH) {
			th_int_usb_vcp_buf_out = 0;
		}
		*c = TH_INT_USB_VCP_ReceiveBuffer[th_int_usb_vcp_buf_out];
		TH_INT_USB_VCP_ReceiveBuffer[th_int_usb_vcp_buf_out] = 0;

		/* Set counters */
		th_int_usb_vcp_buf_out++;
		th_int_usb_vcp_buf_num--;

		/* Data OK */
		return TH_USB_VCP_DATA_OK;
	}
	*c = 0;
	/* Data not ready */
	return TH_USB_VCP_DATA_EMPTY;
}

TH_USB_VCP_Result TH_USB_VCP_Putc(volatile char c) {
	uint8_t ce = (uint8_t)c;

	/* Send data over USB */
	VCP_DataTx(&ce, 1);

	/* Return OK */
	return TH_USB_VCP_OK;
}

TH_USB_VCP_Result TH_USB_VCP_Puts(char* str) {
	while (*str) {
		TH_USB_VCP_Putc(*str++);
	}

	/* Return OK */
	return TH_USB_VCP_OK;
}

uint16_t TH_USB_VCP_Gets(char* buffer, uint16_t bufsize) {
	uint16_t i = 0;
	uint8_t c;

	/* Check for any data on USART */
	if (TH_USB_VCP_BufferEmpty() || (!TH_USB_VCP_FindCharacter('\n') && !TH_USB_VCP_BufferFull())) {
		return 0;
	}

	/* If available buffer size is more than 0 characters */
	while (i < (bufsize - 1)) {
		/* We have available data */
		while (TH_USB_VCP_Getc(&c) != TH_USB_VCP_DATA_OK);
		/* Save new data */
		buffer[i] = (char) c;
		/* Check for end of string */
		if (buffer[i] == '\n') {
			i++;
			/* Done */
			break;
		} else {
			i++;
		}
	}

	/* Add zero to the end of string */
	buffer[i] = 0;

	/* Return number of characters in string */
	return i;
}

TH_USB_VCP_Result TH_INT_USB_VCP_AddReceived(uint8_t c) {
	/* Still available data in buffer */
	if (th_int_usb_vcp_buf_num < USB_VCP_RECEIVE_BUFFER_LENGTH) {
		/* Check for overflow */
		if (th_int_usb_vcp_buf_in >= USB_VCP_RECEIVE_BUFFER_LENGTH) {
			th_int_usb_vcp_buf_in = 0;
		}
		/* Add character to buffer */
		TH_INT_USB_VCP_ReceiveBuffer[th_int_usb_vcp_buf_in] = c;
		/* Increase counters */
		th_int_usb_vcp_buf_in++;
		th_int_usb_vcp_buf_num++;

		/* Return OK */
		return TH_USB_VCP_OK;
	}

	/* Return Buffer full */
	return TH_USB_VCP_RECEIVE_BUFFER_FULL;
}

TH_USB_VCP_Result TH_USB_VCP_GetStatus(void) {
	if (TH_USB_VCP_INT_Init) {
		return TH_USB_VCP_INT_Status;
	}
	return TH_USB_VCP_ERROR;
}

