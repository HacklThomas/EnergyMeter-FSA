#include "main.h"

//--------------------------------------------------------------
// Setzen der Uhrzeit
//-------------------------------------------------------------
void setTime() {
	int eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	TH_RTC.jahr = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	TH_RTC.monat = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	TH_RTC.tag = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	TH_RTC.std = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	TH_RTC.min = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	TH_RTC.sek = eingabe;
	TH_RTC.wotag = 1;
	TH_RTC_SetClock(RTC_DEC);

	return;
}

//--------------------------------------------------------------
// Senden von Daten über UART und USB
//--------------------------------------------------------------
void sendData(char* data) {
	if (interface == BLUETOOTH) {
		TH_UART_SendString(COM1, data, NONE);
	} else {
		TH_USB_VCP_Puts(data);
	}
}

//--------------------------------------------------------------
// Empfangen von Daten über UART und USB
//--------------------------------------------------------------
int receiveData() {
	char data[5];
	if (TH_UART_ReceiveString(COM1, data) == RX_EMPTY) {
		if (TH_USB_VCP_Gets(data, sizeof(data)) == 0) {
			return -1;
		} else {
			interface = USB;
		}
	} else {
		interface = BLUETOOTH;
	}
	return atoi(data);
}

//--------------------------------------------------------------
// Überprüfung, ob sich der Status geändert hat
//--------------------------------------------------------------
bool_t checkForStateChange() {
	int ret = receiveData();
	if (ret == 1) {
		state = INITIALISIEREN;
		return TRUE;
	} else if (ret == 2) {
		state = LISTFILES;
		return TRUE;
	} else if (ret == 3) {
		state = AUSLESEN;
		return TRUE;
	} else if (ret == 4) {
		state = SETTIME;
		return TRUE;
	} else if (ret == 5) {
		state = SETCAN;
		return TRUE;
	} else if (ret == 6) {
		state = BLUETOOTHGRAPH;
		return TRUE;
	} else if (ret == 7) {
		state = DELETEFILES;
		return TRUE;
	} else if (ret == 8) {
		state = TOGGLECAN;
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
// Zählen der Dateien auf der SD-Karte
//--------------------------------------------------------------
int countFiles() {
	int ret = 0;
	char string[40];
	FIL file;
	f_open(&file, "allFiles.txt", FA_READ);
	f_lseek(&file, 0);
	while (f_eof(&file) == 0) {
		f_gets(string, sizeof(string), &file);
		ret++;
	}
	f_close(&file);
	return ret;
}

//--------------------------------------------------------------
// Initialisieren der Peripherie / GPIOs / RTC / Variablen
//--------------------------------------------------------------
void init() {
	SystemInit();
	TH_LED_Init();
	TH_LED_On(LED_RED);
	TH_SYSTICK_Init();
	TH_UART_Init(1);
	TH_RTC_Init();
	TH_USB_VCP_Init();
	TH_CAN1_Init();
	TH_CAN2_Init();
	TH_ADC_Init();

	hcreceive = FALSE;
	TH_UART_SendString(COM1, "AT+NAMEEnergymeter", NONE);

	char data[5];

	/*int i = 0;
	for (i = 0; i < 6; i++) {
		if (i == 0) {
			TH_UART_SendString(COM1, "AT+BAUD9", NONE);
		} else if (i == 1) {
			TH_UART_SendString(COM1, "AT+NAMELOGGER", NONE);
		} else if (i == 2) {
			TH_UART_SendString(COM1, "AT+NAMELOGGER", NONE);
		}
		TH_LED_Toggle(LED_GREEN);
		while (hcreceive == FALSE) {
			TH_LED_On(LED_RED);
		}
		TH_LED_Off(LED_RED);
		hcreceive = FALSE;
		TH_UART_Init(1);
		TH_SYSTICK_Pause_ms(2000);
	}*/
/*
	timer = 0;
	TH_UART_SendString(COM1, "AT+BAUD9", NONE);
	while (hcreceive == FALSE) {
		TH_LED_On(LED_RED);
		if (timer > 2000000) {
			TH_UART_SendString(COM1, "AT+BAUD9", NONE);
		}
	}
	TH_UART_Init(1);
	TH_SYSTICK_Pause_ms(2000);
	timer = 0;
	TH_UART_SendString(COM1, "AT+NAMEDAUMAS", NONE);
	while (hcreceive == FALSE) {
		TH_LED_On(LED_RED);
		if (timer > 2000000) {
			TH_UART_SendString(COM1, "AT+NAMEDAUMAS", NONE);
		}
	}
	TH_LED_Off(LED_RED);
*/
	//TH_UART_Init(1);
	//TH_UART_SendString(COM1, "AT+BAUD8", NONE);

	initIVTMOD();

	blink = 0;
	timer = 0;
	canstate = TRUE;
	return;
}

//--------------------------------------------------------------
// Speichern der gemessenen Daten
//--------------------------------------------------------------
void saveData() {
	static int savecounter = 0;
	char text[64];
	double lowvoltage = (TH_ADC_Read(ADC1, ADC_Channel_10) / 4096.0) * 3 * 48;

	// Daten an Dateiende anhängen
	f_lseek(&fil, f_size(&fil));
	// Zeitdifferenz zum vorigen Wert einfügen
	snprintf(text, sizeof(text), "%ld\t\t", timer);
	timer = 0;
	f_puts(text, &fil);
	snprintf(text, sizeof(text), "%5.3f\t\t%5.3f\t\t%3.2f\t%5.1f\n",
			voltage / 1000.0, current / 1000.0, lowvoltage,
			temperature / 1000.0);
	f_puts(text, &fil);
	// Jede Sekunde werden die Daten zwischengespeichert, um Datenverlust zu verhindern
	savecounter++;
	if (savecounter >= 200) {
		f_sync(&fil);
		savecounter = 0;
	}
}

//--------------------------------------------------------------
// Anelgen einer neuen Datei und Einfügen des Headers
//--------------------------------------------------------------
void insertHeaderToFile() {
	char text[40];
	char buffer[25];
	int temp = 1;
	// Aktuelle Zeit auslesen und in buffer speichern
	TH_RTC_GetClock(RTC_DEC);
	sprintf(buffer, "%02d.%02d.%04d %02d:%02d:%02d", TH_RTC.tag, TH_RTC.monat,
			TH_RTC.jahr + 2000, TH_RTC.std, TH_RTC.min, TH_RTC.sek);
	f_mount(&FatFs, "", 1);
	// Falls es die allg. Datei "allFiles" noch nicht gibt, wird sie neu angelegt
	if (f_open(&fil, "allFiles.txt", FA_CREATE_NEW) == FR_EXIST) {
		f_open(&fil, "allFiles.txt", FA_WRITE);
	}
	// Ermitteln des nächsten Dateinamens
	if (f_size(&fil) > 5) {
		temp = countFiles();
		temp++;
	}
	// Speichern des neuen Dateinamens im allg. File
	snprintf(text, sizeof(text), "%i - %s\n", temp, buffer);
	f_lseek(&fil, f_size(&fil));
	f_puts(text, &fil);
	f_close(&fil);
	// Einfügen allgemeiner Daten in die neue Datei
	sprintf(text, "%i.txt", temp);
	f_open(&fil, text, FA_CREATE_ALWAYS | FA_WRITE);
	f_puts(buffer, &fil);
	f_puts("\n", &fil);
	sprintf(buffer, "Unique ID: 0x%08X 0x%08X 0x%08X\r\n", TH_ID_GetUnique32(0), // LSB
			TH_ID_GetUnique32(1), TH_ID_GetUnique32(2) // MSB
			);
	f_puts(buffer, &fil);
	f_puts("Time\t\tVoltage\t\tCurrent\t\tLV\tTemperature\n", &fil);
	return;
}

//--------------------------------------------------------------
// Verschlüsseln der Daten
//--------------------------------------------------------------
void encrypt_cbc() {
	static uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
			0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	static uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	uint8_t in[64];

	int i = 0;
	for (i = 0; i < 64; i++) {
		in[i] = buffer[i];
	}

	AES128_CBC_encrypt_buffer(cyphertext, buffer, 64, key, iv);
}

//--------------------------------------------------------------
// Senden des Kryptotestes im HEX Format
//--------------------------------------------------------------
void sendCyphertext() {
	char temp[4];
	char hexstring[195];
	for (int i = 0; i < 64; i++) {
		sprintf(temp, "%2x ", cyphertext[i]);
		strcat(hexstring, temp);
	}
	sendData("CRYPT: \n");
	sendData(hexstring);
	sendData("\n");
}

//--------------------------------------------------------------
// Senden der Daten über CAN (Errorbits / Spannung / Strom / Temperatur)
//--------------------------------------------------------------
void sendCAN() {
	CAN1_TX_FRAME_t TXFrame;

	TXFrame.can_id = 0x430;
	TXFrame.anz_bytes = 8;
	TXFrame.data[0] = 0;
	TXFrame.data[1] = 0;
	TXFrame.data[2] = 5; //(uint8_t)((voltage / 1000.0) / 4);
	TXFrame.data[3] = (uint8_t)(current / 1000.0);
	TXFrame.data[4] = (uint8_t)(temperature / 1000.0);

	TH_CAN1_send_std_data(TXFrame);
}

//--------------------------------------------------------------
// Initialisieren des IVT-MOD
//--------------------------------------------------------------
void initIVTMOD() {
	sendFrameToIVTMOD(0x34, 0x00, 0x01, 0x00); // STOP

	sendFrameToIVTMOD(0x20, 0x02, 0x00, 0x05); // Conf I: cyclic 5ms
	sendFrameToIVTMOD(0x21, 0x02, 0x00, 0x05); // Conf U1: cyclic 5ms
	sendFrameToIVTMOD(0x24, 0x02, 0x13, 0x88); // Conf T: cyclic 5s

	sendFrameToIVTMOD(0x22, 0x00, 0x00, 0x00); // Disable U2
	sendFrameToIVTMOD(0x23, 0x00, 0x00, 0x00); // Disable U3
	sendFrameToIVTMOD(0x25, 0x00, 0x00, 0x00); // Disable P
	sendFrameToIVTMOD(0x26, 0x00, 0x00, 0x00); // Disable E / As
	sendFrameToIVTMOD(0x27, 0x00, 0x00, 0x00); // Disable E / Wh

	sendFrameToIVTMOD(0x32, 0x00, 0x00, 0x00); // STORE
	sendFrameToIVTMOD(0x34, 0x01, 0x01, 0x00); // START
}

void sendFrameToIVTMOD(uint8_t db0, uint8_t db1, uint8_t db2, uint8_t db3) {
	CAN2_TX_FRAME_t transmit;

	transmit.anz_bytes = 8;
	transmit.can_id = 0x411;

	transmit.data[0] = db0;
	transmit.data[1] = db1;
	transmit.data[2] = db2;
	transmit.data[3] = db3;
	transmit.data[4] = 0x00;
	transmit.data[5] = 0x00;
	transmit.data[6] = 0x00;
	transmit.data[7] = 0x00;

	TH_CAN2_send_std_data(transmit);
	TH_SYSTICK_Pause_ms(5);
}

