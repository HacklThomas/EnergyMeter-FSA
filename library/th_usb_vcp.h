//--------------------------------------------------------------
// File        : th_usb_vcp.h
// Description : With this library, your STM32F4xx will be seen to your computer as Virtual COM Port (VCP).
//               To be able to work, you have to install ST's VCP Driver, from link below:
//						http://www.st.com/web/en/catalog/tools/PF257938
// Data +	   : PA12
// Data -	   : PA11
//--------------------------------------------------------------

#ifndef TH_USB_VCP_H
#define TH_USB_VCP_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "defines.h"
/* Parts of USB device */
#include "usbd_cdc_core.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------

// Increase this value if you need more memory for VCP receive data
#ifndef USB_VCP_RECEIVE_BUFFER_LENGTH
#define USB_VCP_RECEIVE_BUFFER_LENGTH		128
#endif


//VCP Result Enumerations
typedef enum {
	TH_USB_VCP_OK,                  /*!< Everything ok */
	TH_USB_VCP_ERROR,               /*!< An error occurred */
	TH_USB_VCP_RECEIVE_BUFFER_FULL, /*!< Receive buffer is full */
	TH_USB_VCP_DATA_OK,             /*!< Data OK */
	TH_USB_VCP_DATA_EMPTY,          /*!< Data empty */
	TH_USB_VCP_NOT_CONNECTED,       /*!< Not connected to PC */
	TH_USB_VCP_CONNECTED,           /*!< Connected to PC */
	TH_USB_VCP_DEVICE_SUSPENDED,    /*!< Device is suspended */
	TH_USB_VCP_DEVICE_RESUMED       /*!< Device is resumed */
} TH_USB_VCP_Result;

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------

/**
 * @brief  Initializes USB VCP
 * @param  None
 * @retval TM_USB_VCP_OK
 */
TH_USB_VCP_Result TH_USB_VCP_Init(void);

/**
 * @brief  Gets received character from internal buffer
 * @param  *c: pointer to store new character to
 * @retval Character status:
 *            - TM_USB_VCP_DATA_OK: Character is valid inside *c_str
 *            - TM_USB_VCP_DATA_EMPTY: No character in *c
 */
TH_USB_VCP_Result TH_USB_VCP_Getc(uint8_t* c);

/**
 * @brief  Puts character to USB VCP
 * @param  c: character to send over USB
 * @retval TM_USB_VCP_OK
 */
TH_USB_VCP_Result TH_USB_VCP_Putc(volatile char c);

/**
 * @brief  Gets string from VCP port
 *
 * @note   To use this method, you have to send \n (0x0D) at the end of your string,
 *         otherwise data can be lost and you will fall in infinite loop.
 * @param  *buffer: Pointer to buffer variable where to save string
 * @param  bufsize: Maximum buffer size
 * @retval Number of characters in buffer:
 *            - 0: String not valid
 *            - > 0: String valid, number of characters inside string
 */
uint16_t TH_USB_VCP_Gets(char* buffer, uint16_t bufsize);

/**
 * @brief  Puts string to USB VCP
 * @param  *str: Pointer to string variable
 * @retval TM_USB_VCP_OK
 */
TH_USB_VCP_Result TH_USB_VCP_Puts(char* str);

/**
 * @brief  Gets VCP status
 * @param  None
 * @retval Device status:
 *            - TM_USB_VCP_CONNECTED: Connected to computer
 *            - other: Not connected and not ready to communicate
 */
TH_USB_VCP_Result TH_USB_VCP_GetStatus(void);

/**
 * @brief  Checks if receive buffer is empty
 * @param  None
 * @retval Buffer status:
 *            - 0: Buffer is not empty
 *            - > 0: Buffer is empty
 */
uint8_t TH_USB_VCP_BufferEmpty(void);

/**
 * @brief  Checks if receive buffer is fukk
 * @param  None
 * @retval Buffer status:
 *            - 0: Buffer is not full
 *            - > 0: Buffer is full
 */
uint8_t TH_USB_VCP_BufferFull(void);

/**
 * @brief  Checks if character is in buffer
 * @param  c: Character to be checked if available in buffer
 * @retval Character status:
 *            - 0: Character is not in buffer
 *            - > 0: Character is in buffer
 */
uint8_t TH_USB_VCP_FindCharacter(volatile char c);

//--------------------------------------------------------------
// Interne Funktionen
//--------------------------------------------------------------
extern TH_USB_VCP_Result TH_INT_USB_VCP_AddReceived(uint8_t c);


#endif // TH_USB_VDC_H

