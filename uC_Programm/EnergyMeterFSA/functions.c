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
	} else if (ret == DELETECODE) {
		state = DELETEFILES;
		return TRUE;
	} else if (ret == 8) {
		state = TOGGLECAN;
		return TRUE;
	} else if (ret == 9) {
		state = DELETEINITFILE;
		return TRUE;
	} else if (ret == 10) {
		state = TEST;
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
	TH_RTC_Init();
	TH_USB_VCP_Init();
	TH_CAN1_Init();
	TH_CAN2_Init();
	TH_ADC_Init();
	TH_RNG_Init();
	TH_CRC_Init();

	hcreceive = FALSE;

	initIVTMOD();

	CAN2_RX_FRAME_t rxframe;

	TH_LED_Off(LED_RED);

	int i = 0;
	int serial = 0;

	f_mount(&FatFs, "", 1);

	// Konfigurieren des Bluetooth Moduls (nur beim allerersten Programmstart)
	if (f_open(&initfile, "initfile.txt", FA_CREATE_NEW) != FR_EXIST) {
		// BLUTOOTH MODULE CONFIG
		TH_UART_Init(0);
		TH_LED_On(LED_RED);
		f_close(&initfile);

		timer = 0;
		TH_UART_SendString(COM1, "AT+BAUD9", NONE);
		while (hcreceive == FALSE) {
			TH_LED_On(LED_RED);
			if (timer > 2000000) {
				i++;
				TH_UART_SendString(COM1, "AT+BAUD9", NONE);
				timer = 0;
			}
			if (i > 4) {
				i = 0;
				break;
			}
		}
		TH_LED_Off(LED_RED);
		TH_UART_Init(1);
		TH_SYSTICK_Pause_ms(2000);
		timer = 0;
		TH_UART_SendString(COM1, "AT+NAMEEnergyMeter1", NONE);
		while (hcreceive == FALSE) {
			TH_LED_On(LED_RED);
			if (timer > 2000000) {
				i++;
				TH_UART_SendString(COM1, "AT+NAMEEnergyMeter1", NONE);
				timer = 0;
			}
			if (i > 4) {
				break;
			}
		}
		TH_LED_Off(LED_RED);
	}
	TH_UART_Init(1);

	// Speichern der IVT MOD Seriennummer in File (nur beim allerersten Programmstart)
	if (f_open(&initfile, "ivtserial.txt", FA_CREATE_ALWAYS | FA_WRITE)
			== FR_OK) {
		if (f_size(&initfile) < 4) {
			do {
				sendFrameToIVTMOD(0x7B, 0x00, 0x00, 0x00); // Get Serial Number
				TH_CAN2_receive(&rxframe);
			} while (rxframe.data[0] != 0xBB);
			serial = (rxframe.data[1] * 0x1000000) + (rxframe.data[2] * 0x10000)
					+ (rxframe.data[3] * 0x100) + (rxframe.data[4]);
			sprintf(buffer, "%d\n", serial);
			f_puts("IVT MOD Serial Number: ", &initfile);
			f_puts(buffer, &initfile);
		}
	}
	f_close(&initfile);

	// Check ob CAN aktiviert ist
	if (f_open(&initfile, "canactive.txt", FA_OPEN_EXISTING) != FR_OK) {
		canstate = FALSE;
	} else {
		canstate = TRUE;
		f_close(&initfile);
	}

	f_mount(0, "", 1);

	blink = 0;
	timer = 0;
	ivterror = FALSE;
	syncing = FALSE;

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
	//snprintf(text, sizeof(text), "%5.3f\t\t%5.3f\t\t%3.2f\t%5.1f\n", voltage / 1000.0, current / 1000.0, lowvoltage, temperature / 1000.0);
	snprintf(text, sizeof(text), "%5.3f\t\t%5.3f\t\t%3.2f\n", voltage / 1000.0,
			current / 1000.0, lowvoltage);
	f_puts(text, &fil);
	// Alle 600 Werte werden die Daten zwischengespeichert, um Datenverlust zu verhindern
	savecounter++;
	if (savecounter >= 600) {
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
	char serial[35];
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

	// Auslesen der IVT MOD Seriennummer
	f_open(&fil, "ivtserial.txt", FA_READ);
	f_gets(serial, sizeof(serial), &fil);
	f_close(&fil);

	// Einfügen allgemeiner Daten in die neue Datei
	sprintf(text, "%i.txt", temp);
	f_open(&fil, text, FA_CREATE_ALWAYS | FA_WRITE);
	f_puts(buffer, &fil);
	f_puts("\n", &fil);
	sprintf(buffer, "\nuC unique ID: 0x%08X 0x%08X 0x%08X\r\n",
			TH_ID_GetUnique32(0), // LSB
			TH_ID_GetUnique32(1), TH_ID_GetUnique32(2) // MSB
			);
	f_puts(buffer, &fil);
	f_puts(serial, &fil);
	//f_puts("\nTime\t\tVoltage\t\tCurrent\t\tLV\tTemperature\n", &fil);
	f_puts("\nTime\t\tVoltage\t\tCurrent\t\tLV\n", &fil);
	return;
}

//--------------------------------------------------------------
// Verschlüsseln der Daten
//--------------------------------------------------------------
void encrypt_cbc(uint32_t input) {
	static uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
			0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	static uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	uint8_t in[4];

	in[0] = (input & 0xFF000000) >> 24;
	in[1] = (input & 0x00FF0000) >> 16;
	in[2] = (input & 0x0000FF00) >> 8;
	in[3] = input & 0x000000FF;

	for (int i = 0; i < 4; i++) {
		cyphertext[i] = 0x00;
	}

	AES128_CBC_encrypt_buffer(cyphertext, in, 4, key, iv);
	sendCyphertext();
	return;
}

//--------------------------------------------------------------
// Senden des Kryptotestes im HEX Format
//--------------------------------------------------------------
void sendCyphertext() {
	char temp[4];
	char hexstring[50];
	hexstring[0] = 0;
	for (int i = 0; i < 16; i++) {
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
	if (ivterror) {
		TXFrame.data[1] = 1;
	} else {
		TXFrame.data[1] = 0;
	}
	TXFrame.data[2] = (uint8_t) ((voltage / 1000.0) / 4);
	TXFrame.data[3] = (uint8_t) (current / 1000.0);
	TXFrame.data[4] = (uint8_t) (temperature / 1000.0);

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

