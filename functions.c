#include "main.h"

//--------------------------------------------------------------
// Setzen der Uhrzeit
//-------------------------------------------------------------
void setTime() {
	int eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	UB_RTC.jahr = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	UB_RTC.monat = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	UB_RTC.tag = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	UB_RTC.std = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	UB_RTC.min = eingabe;
	do {
		eingabe = receiveData();
	} while (eingabe == -1);
	UB_RTC.sek = eingabe;
	UB_RTC.wotag = 1;
	UB_RTC_SetClock(RTC_DEC);
	return;
}

//--------------------------------------------------------------
// Senden von Daten über UART und USB
//--------------------------------------------------------------
void sendData(char* data) {
	if (interface == BLUETOOTH) {
		Uart_SendString(COM1, data, NONE);
	} else {
		TM_USB_VCP_Puts(data);
	}
}

//--------------------------------------------------------------
// Empfangen von Daten über UART und USB
//--------------------------------------------------------------
int receiveData() {
	char data[5];
	if (Uart_ReceiveString(COM1, data) == RX_EMPTY) {
		if (TM_USB_VCP_Gets(data, sizeof(data)) == 0) {
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
	} else if (ret == 2) {
		state = LISTFILES;
	} else if (ret == 3) {
		state = AUSLESEN;
	} else if (ret == 4) {
		state = SETTIME;
	} else if (ret == 6) {
		state = BLUETOOTHGRAPH;
	} else if (ret == 7) {
		state = DELETEFILES;
	} else if (ret == 8) {
		state = TOGGLECAN;
	} else {	
		return FALSE;
	}
	return TRUE;
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
	UB_RTC_Init();
	Led_Init();
	UB_CAN1_Init();
	UB_CAN2_Init();
	TM_USB_VCP_Init();
	UB_Systick_Init();
	
	UB_Uart_Init(0);
	Uart_SendString(COM1, "AT+BAUD9", NONE);
	UB_Uart_Init(1);
	Uart_SendString(COM1, "AT+BAUD9", NONE);
	
	TM_ADC_InitADC(ADC1);
	TM_ADC_Init(ADC1, ADC_Channel_10);
	TM_ADC_EnableVbat();
	initIVTMOD();
	
	timer = 0;
	oldtimerval = 0;
	canactive = TRUE;

	return;
}

//--------------------------------------------------------------
// Speichern der gemessenen Daten
//--------------------------------------------------------------
void saveData() {
	static int savecounter = 0;
	char text[15];
	uint32_t difference;
	
	// Daten an Dateiende anhängen
	f_lseek(&fil, f_size(&fil));
	// Zeitdifferenz zum vorigen Wert einfügen
	difference = timer - oldtimerval;
	oldtimerval = timer;
	snprintf(text, sizeof(text), "%4ld\t\t", difference);
	f_puts(text, &fil);
	snprintf(text, sizeof(text), "%5.3f\t\t", voltage / 1000.0);
	f_puts(text, &fil);
	snprintf(text, sizeof(text), "%5.3f\t\t", current / 1000.0);
	f_puts(text, &fil);
	double lowvoltage = (TM_ADC_Read(ADC1, ADC_Channel_10) / 4096.0) * 3.3 * 48;
	snprintf(text, sizeof(text), "%3.2f\t", lowvoltage);
	f_puts(text, &fil);
	double vbat = (TM_ADC_ReadVbat(ADC1) / 4096.0) * 3.3;
	snprintf(text, sizeof(text), "%3.2f\t", vbat);
	f_puts(text, &fil);
	snprintf(text, sizeof(text), "%5.1f\n", temperature / 1000.0);
	f_puts(text, &fil);
	
	// Jede Sekunde werden die Daten zwischengespeichert, um Datenverlust zu verhindern
	savecounter++;
	if (savecounter >= 200) {
		f_sync(&fil);
		savecounter = 0;
	}
}

//--------------------------------------------------------------
// Anlegen einer neuen Datei und Einfügen des Headers
//--------------------------------------------------------------
void insertHeaderToFile() {
	char text[40];
	char buffer[25];
	int temp = 1;
	// Aktuelle Zeit auslesen und in buffer speichern
	UB_RTC_GetClock(RTC_DEC);
	sprintf(buffer, "%02d.%02d.%04d %02d:%02d:%02d", UB_RTC.tag, UB_RTC.monat,
			UB_RTC.jahr + 2000, UB_RTC.std, UB_RTC.min, UB_RTC.sek);
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
	// Einfügen des Headers in die neue Datei
	sprintf(text, "%i.txt", temp);
	f_open(&fil, text, FA_CREATE_ALWAYS | FA_WRITE);
	f_puts(buffer, &fil);
	f_puts("\n", &fil);
	sprintf(buffer, "Unique ID: 0x%08X 0x%08X 0x%08X\r\n", TM_ID_GetUnique32(0), TM_ID_GetUnique32(1), TM_ID_GetUnique32(2));
	f_puts(buffer, &fil);
	f_puts("Time\t\tVoltage\t\tCurrent\t\tLV\t\tVbat\tTemperature\n", &fil);
	return;
}

//--------------------------------------------------------------
// Verschlüsseln der Daten
//--------------------------------------------------------------
void encrypt_cbc(uint8_t in[64]) {
	uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab,
			0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
			0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

	AES128_CBC_encrypt_buffer(cyphertext, in, 64, key, iv);
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
	static int cancounter = 0;
	
	if (canactive == TRUE) {
		cancounter++;
		if (cancounter >= 20) {
			TXFrame.data[0] = 0;
			TXFrame.data[1] = 0;
			TXFrame.data[2] = (uint8_t)((voltage / 1000.0) / 4);
			TXFrame.data[3] = (uint8_t)(current / 1000.0);
			TXFrame.data[4] = (uint8_t)(temperature / 1000.0);

			TXFrame.can_id = 0x430;
			TXFrame.anz_bytes = 8;
			UB_CAN1_send_std_data(TXFrame);

			cancounter = 0;
		}
	}
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

	UB_CAN2_send_std_data(transmit);
	UB_Systick_Pause_ms(5);
}
