#include "main.h"

/* ALPHA Prototyp:
 * IVT-A:    SPI1 PinsPack1 | EOC: PE3 | OCS: PE2 | SS: PA4
 * SD-Card:  SPI3 PinsPack2 | CS: PA15
 * BT-Modul: RX: PA9 | TX: PA10 = USART1 PinsPack1
 *
 * BETA Prototyp:
 * IVT-A:    SPI2 PinsPack2 | EOC: PB10 | OCS: PB11 | SS: PB12
 * SD-Card:  SPI1 PinsPack1 | CS: PA4
 * BT-Modul: RX: PC10 | TX: PC11 = USART3 PinsPack2
 */

int main(void) {
	init();

	state = INITIALISIEREN;
	interface = UNDEFINED;

	uint32_t pointer = 0;
	int ret = 5;
	int temp = 0;
	bool_t prevcanstate;

	for (int i = 0; i < 64; i++) {
		cyphertext[i] = 0x00;
	}

	while (1) {
		switch (state) {
		case INITIALISIEREN: // File wird angelegt
			insertHeaderToFile();
			state = LOGGING;
			break;
		case LOGGING: // Die eigentliche Datenspeicherung erfolgt im CAN-Interrupt -> Lib CAN2 ganz unten
			if (checkForStateChange() == TRUE) {
				f_close(&fil);
				f_mount(0, "", 1);
			}
			break;
		case LISTFILES: // Alle gesp. Filenamen werden ausgegeben
			prevcanstate = canstate; // CAN wird während der Übertragung deaktiviert
			canstate = FALSE;
			Led_On(LED_RED);
			f_mount(&FatFs, "", 1);
			temp = countFiles();
			f_open(&fil, "allFiles.txt", FA_READ);
			f_lseek(&fil, 0);
			for (int i = 1; i <= temp; i++) {
				f_gets(buffer, sizeof(buffer), &fil);
				pointer = f_tell(&fil);
				f_close(&fil);
				ret = strlen(buffer);
				ret--;
				buffer[ret] = 0x00;
				sendData(buffer);
				sendData(" - Size (Bytes): ");
				sprintf(buffer, "%i.txt", i);
				f_open(&fil, buffer, FA_READ);
				itoa(f_size(&fil), buffer, 10);
				sendData(buffer);
				sendData("\n");
				f_close(&fil);
				f_open(&fil, "allFiles.txt", FA_READ);
				f_lseek(&fil, pointer);
			}
			sendData("ENDE"); // Endekennung für Ausleseprogramm
			f_close(&fil);
			f_mount(0, "", 1);
			Led_Off(LED_RED);
			state = IDLE;
			canstate = prevcanstate;
			break;
		case AUSLESEN: // Auslesen eines bestimmmten Files
			prevcanstate = canstate;
			canstate = FALSE;
			f_mount(&FatFs, "", 1);
			Led_On(LED_RED);
			do {
				ret = receiveData();
			} while (ret == -1);
			sprintf(buffer, "%i.txt", ret);
			f_open(&fil, buffer, FA_READ);
			f_lseek(&fil, 0);
			while (f_eof(&fil) == 0) {
				f_gets(buffer, sizeof(buffer), &fil);
				sendData(buffer);
				temp++;
				encrypt_cbc(); // Der Schlüssel wird über 200 Zeilen berechnet und dann ausgegeben
				if (temp > 200) {
					temp = 0;
					sendCyphertext();
					for (int i = 0; i < 64; i++) {
						cyphertext[i] = 0x00;
					}
				}
			}
			sendCyphertext();
			temp = 0;
			sendData("ENDE");
			f_close(&fil);
			f_mount(0, "", 1);
			state = IDLE;
			Led_Off(LED_RED);
			for (int i = 0; i < 64; i++) {
				cyphertext[i] = 0x00;
			}
			canstate = prevcanstate;
			break;
		case SETTIME: // Einstellen der Uhrzeit für die RTC
			Led_On(LED_RED);
			setTime();
			state = IDLE;
			Led_Off(LED_RED);
			break;
		case BLUETOOTHGRAPH: // Mit einer Android App können die Daten in Echtzeit visualisiert werden
			// SEQUENCE: "E902.0,123.4,193.43\n"
			Led_On(LED_RED);
			sprintf(buffer, "E%5.2lf,%5.2lf,%5.2lf\n", current / 1000.0,
					voltage / 1000.0, temperature / 1000.0);
			Uart_SendString(COM1, buffer, NONE);
			if (checkForStateChange()) {
				Led_Off(LED_RED);
			}
			break;
		case DELETEFILES: // Löschen aller Files auf der SD Karte
			Led_On(LED_RED);
			f_mount(&FatFs, "", 1);
			f_findfirst(&dj, &fno, "", "*.txt");
			while (fno.fname[0]) {
				f_unlink(fno.fname);
				f_findnext(&dj, &fno);
			}
			f_mount(0, "", 1);
			insertHeaderToFile();
			f_close(&fil);
			f_mount(0, "", 1);
			Led_Off(LED_RED);
			state = IDLE;
			break;
		case IDLE: // Warten auf nächsten Befehl
			Led_On(LED_ORANGE);
			if (checkForStateChange()) {
				Led_Off(LED_ORANGE);
			}
			break;
		case TOGGLECAN: // De-/Aktivieren der CAN Ausgabe
			if (canstate == TRUE) {
				canstate = FALSE;
			} else {
				canstate = TRUE;
			}
			state = IDLE;
			break;
		case TEST:
			break;
		default:
			break;
		}
	}
}

